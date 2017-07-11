var width = 960, height = 800, node, link, root, canvas;
var graphModel = null;
var force = d3.layout.force()
	.linkDistance(200)
	.size([ width, height ])
	.on('tick', tick);
var fill = d3.scale.category10();
var currently_selected_node = null;
var linkedByIndex = {};
var imageLocation = 'static/img/';
var selnode = null;
var ctrlScope = null;

// 1x1 mapping of node type to associated icon
var nodeTypeImage = {
		'master'  : 'aggregator.png',
		'slave'   : 'breaker.jpg',
		'default' : 'pc_tower.png'
};

function isInArray(value, array) {
	return array.indexOf(value) > -1;
}

function isConnected(a, b) {
	return linkedByIndex[a.index + ',' + b.index]
			|| linkedByIndex[b.index + ',' + a.index];
}

function mouseovered(d) {
	currently_selected_node = d3.select(this);
	node.classed('node-active', function(o) {
		thisOpacity = isConnected(d, o) ? true : false;
		this.setAttribute('fill-opacity', thisOpacity);
		return thisOpacity;
	});

	link.classed('link-active', function(o) {
		return o.source === d || o.target === d ? true : false;
	});
	d3.select(this).classed('node-active', true);
//	d3.select(this).select('circle').transition().duration(500).attr('r',
//			d.weight).style('opacity', d.node_size);
}

// mouse - fly over nodes
function mouseouted(d) {
	currently_selected_node = null;
	node.classed('node-active', false);
	link.classed('link-active', false);
	d3.select(this).classed('node-active', false);

	d3.select(this).select('node').transition().duration(500).attr('r', 5)
			.style('opacity', 1);
}

function autocompletesearch(optArray) {
//	console.log(optArray);
	$('#searchinput').autocomplete(
			{
				source : function(request, response) {
					var matches = $
							.map(optArray,
									function(acItem) {
										if (acItem.toUpperCase()
												.indexOf(request.term.toUpperCase()) === 0) {
											return acItem;
										}
									});
					response(matches);
				},
				messages : {
					noResults : '',
					results : function() {
					}
				},
				response : function(event, ui) {
					if (currently_selected_node) {
						currently_selected_node.each(mouseouted);
					}
					var result = [];
					for (var i = 0; i < ui.content.length; i++) {
						result.push(ui.content[i].value);
					}
					var not_matching = canvas.selectAll('.node').filter(function(d, i) {
						return isInArray(d.name, result) == false;
					})

					if (result.length == 1) {
						var matching = canvas.selectAll('.node').filter(function(d, i) {
							return isInArray(d.name, result) == true;
						})
						matching.each(mouseovered);
					} else {
						not_matching.style('opacity', 0);
						var link = canvas.selectAll('.link')
						link.style('opacity', '0');
						d3.selectAll('.node, .link').transition().duration(5000).style(
								'opacity', 1);
					}
				}
			})
}

function tick() {
  var h = height;
  var w = width;
  link.attr("x1", function(d) { return d.source.x; })
    .attr("y1", function(d) { return d.source.y; })
    .attr("x2", function(d) { return d.target.x; })
    .attr("y2", function(d) { return d.target.y; });

  node.attr("transform", function(d) {
    return "translate(" + Math.max(5, Math.min(w - 5, d.x)) + "," + Math.max(5, Math.min(h - 5, d.y)) + ")";
  });
};

function dict_values(dict) {
	return Object.keys(dict).map(function(key) {
		return dict[key];
	});
};

function isVisible(d) {
	return d.visible;
}

function parse_json_col(jdict) {
		var gModel = {
				'nodes':[],
				'links':[],
				'protocols':[],
				'functions':[],
				'targets':[],
				'protocolCnt':{'_total':0}};
		root = jdict;
		root.fixed = true;
		root.x = width / 2;
		root.y = height / 2 - 80;

		if (root.nodes == null) {
			alert("\nGraph cannot be plotted\n\nNo devices were found for that time period.");
			return;
		}
		
		// Add node search feature
		var optArray = [];
		for (var i = 0; i < root.nodes.length - 1; i++) {
			if (root.nodes[i]['name'] != '-')
				optArray.push(root.nodes[i]['name']);
		}
		optArray = optArray.sort();
		$(function() {
			autocompletesearch(optArray);
		});
	
		if (root.globals.hasOwnProperty('changes')) {
			gModel['changes'] = root.globals['changes'];
		}
		if (root.globals.hasOwnProperty('protocols')) {
			gModel['protocols'] = root.globals['protocols'];
		}
		if (root.globals.hasOwnProperty('functions')) {
			gModel['functions'] = root.globals['functions'];
		}
		if (root.globals.hasOwnProperty('targets')) {
			gModel['targets'] = root.globals['targets'];
		}
		
		console.log(root.nodes.length + ' nodes and ' + root.links.length
				+ ' links and globals '+JSON.stringify(root.globals));
		gModel['links'] = [];
		gModel['nodes'] = [];
		
		// Give nodes ids and initialize variables
		for (var i = 0; i < root.nodes.length; i++) {
			var node = root.nodes[i];
			if (node.name == '-') {
				continue;
			}
			gModel['nodes'].push(node)
			node.visible = true;
		}
		// Give links ids and initialize variables
		for (var i = 0; i < root.links.length; i++) {
			var link = root.links[i];
			if (link.source_name == '-' | link.target_name == '-') {
				continue;
			}
			gModel['links'].push(link);
			if (link.PROTOCOL.length > 0) {
				link.snode = root.nodes[link.source];
				link.tnode = root.nodes[link.target];
				link.visible = true;
				// Find first non empty protocol (baseline might have '-')
				lnk_proto = link.PROTOCOL[0]['name']
				for (var j = 0; j < link.PROTOCOL.length; j++) {
					if (link.PROTOCOL[j]['name'] != '-') {
						lnk_proto = link.PROTOCOL[j]['name'];
						break;
					}
				}
				if (lnk_proto in gModel['protocolCnt']) {
					gModel['protocolCnt'][lnk_proto] = gModel['protocolCnt'][lnk_proto] + link.value;
				}
				else {
					gModel['protocolCnt'][lnk_proto] = link.value;
				}
				gModel['protocolCnt']['_total'] += link.value;
				
			}
		}
		return gModel;
}
function clearCanvas() {
	canvas.selectAll('*').remove();
	var canvasHdr = document.getElementById('canvasState');
	while (canvasHdr.firstChild) {
		canvasHdr.removeChild(canvasHdr.firstChild);
	}
}

function graphSwitchLabelDisplay(show) {
	// Show/Hide node labels
	if (!show) {
		 d3.selectAll('.node_label').style('opacity', 0);
	}
	else {
		 d3.selectAll('.node_label').style('opacity', 1);		
	}
}

function buildGraph($scope, json_col_dict) {
	console.log('Building graph...');
	ctrlScope = $scope;
//	$scope.$watch('selectedNode', function(newValue, oldValue) {
//		if ($scope.selectedNode != null) {
//	    console.log('selected node is: '+$scope.selectedNode);}
//		});
	// Extract nodes and links info from json dict
	graphModel = parse_json_col(json_col_dict);
	if (graphModel == null) {
		return;
	}
	// Build a dictionary with all links to/from every node
	// Will be used to display node details
	$scope.nodeLinks = graphModel['links']

	var protocolColors = {};
	var legend_data = {labels: [], datasets: [{data:[], backgroundColor: [], hoverBackgroundColor: []}]};
	for (var i = 0; i < graphModel.protocols.length; i++) {
		proto = graphModel.protocols[i];
		clr = '#' + Math.random().toString(16).slice(2, 8);
		protocolColors[proto] = clr;
		legend_data.labels.push(proto);
		legend_data.datasets[0].data.push(Math.round(graphModel['protocolCnt'][proto]/graphModel['protocolCnt']['_total']*100));
		legend_data.datasets[0].backgroundColor.push(clr);
		legend_data.datasets[0].hoverBackgroundColor.push(clr);
		}

	var legend_options = {
    animation:{
        animateScale:true
    },
    legend: {
      display: false,
      position: 'right',
      labels: {
          fontSize: 12
      }
    },
    tooltips: {
      callbacks: {
        label: function(tooltipItem, data) {
          //get the concerned dataset
          var dataset = data.datasets[tooltipItem.datasetIndex];
          //calculate the total of this data set
          var total = dataset.data.reduce(function(previousValue, currentValue, currentIndex, array) {
            return previousValue + currentValue;
          });
          //get the current items value
          var currentValue = dataset.data[tooltipItem.index];
          //calculate the precentage based on the total and current item, also this does a rough rounding to give a whole number
          var precentage = Math.floor(((currentValue/total) * 100)+0.5);

          return precentage + "%";
        }
      }
    } 
	};
	if (document.getElementById("myChart")) {
		console.log(JSON.stringify(legend_data));
		var ctx = document.getElementById("myChart").getContext("2d");
		var myDoughnutChart = new Chart(ctx, {
	    type: 'doughnut',
	    data: legend_data,
	    options: legend_options
		});
		document.getElementById('js-legend').innerHTML = myDoughnutChart.generateLegend();		
	}

	// Show all nodes and links
	clearCanvas();

	var k = Math.sqrt(root.nodes.length / (width * height));

	// Drag feature
	var node_drag = d3.behavior.drag()
	.on('dragstart', dragstarted)
	.on('drag', dragged)
	.on('dragend', dragended);

	// Start the layout
	force.charge(-10 / k)
		.gravity(100 * k)
		.nodes(root.nodes)
		.links(root.links)
		.start()

	// Update the links
		link = canvas
			.selectAll('link')
			.data(root.links.filter(function(d) {
				return d.source_name != '-' && d.target_name != '-';
			}))
			.enter()
			.insert("svg:line", ".node")
	    .attr("class", "link")
	    .attr("x1", function(d) { return d.source.x; })
	    .attr("y1", function(d) { return d.source.y; })
	    .attr("x2", function(d) { return d.target.x; })
	    .attr("y2", function(d) { return d.target.y; })
	    .attr('id',function(d,i) { return 'linkId_' + i; })
			.attr('visible', true)
			.style('stroke', function(d) {
				// Color link by protocol
				if (d['PROTOCOL'].length > 0) {
					lnk_color = protocolColors[d['PROTOCOL'][0]['name']];
					return lnk_color;					
				}
			})
			.style('stroke-width', function(d) {
				// Link width depending on bandwidth
				lnk_width = 1 + Math.atan2(d.value / 100, 1) * 2;
				 return lnk_width;
			});
	
	link.append("title").text("link");
	link.on('contextmenu',linkpopup)

	var labelText = canvas.selectAll('.labelText')
  .data(root.links)
.enter().append('text')
  .attr('class','labelText')
  .attr('dx',20)
  .attr('dy',0)
  .style('fill','red')
.append('textPath')
  .attr('xlink:href',function(d,i) { return '#linkId_' + i;})
  .text(function(d,i) { return 'text for link ' + i;});
	
	// Update the nodes
	node = canvas
	.selectAll('circle.node')
	.data(root.nodes.filter(function(d) {
				return d.name != '-';
			}))

	node.transition()
    .attr("r", function(d) { return d.children ? 3.5 : Math.pow(d.size, 2/5) || 1; });

	node.enter()
	.append('defs')
		.append('pattern')
			.attr('id', function(d) {return (d.id+'-icon');})
			.attr('width', 1)
			.attr('height', 1)
			.attr('patternContentUnits', 'objectBoundingBox')
			.append('svg:image')
				.attr('xlink:href', function (d) {
						if ($.inArray(d.node_type, Object.keys(nodeTypeImage)) != -1) {
						return imageLocation+nodeTypeImage[d.node_type];
						}
						else {
							return imageLocation+nodeTypeImage['default'];
						}
				})
				.attr('x', 0)
				.attr('y', 0)
				.attr('height', 1)
				.attr('width', 1)
				.attr("preserveAspectRatio", "xMinYMin slice");

	 var vertex = node.enter()
	 	.append('g')
	 	.attr('class', 'draggable node')
	 //	.call(dragGroup);
	 	.call(node_drag)
		.on('dblclick', releasenode)
		.on('mouseover', mouseovered)
		.on('mouseout', mouseouted)
		.on('contextmenu',nodepopup);
	 
	// Add circle around node
 	vertex.append('svg:circle')
 		.attr('stroke',function(d) {
 			if (d.diffs) {
 				if (d.diffs[0]['diff'] == 'inserted') {
 					return 'green';
 				}
 				else if (d.diffs[0]['diff'] == 'deleted') {
 					return 'red';
 				}
 				else {
 					return 'orange';
 				}
 				
 			}
 			return 'lightgrey';
 		})
	 	.attr('r', function(d) {
	 		orig_size = d.size
	 		if (d.size == 1) {
	 			orig_size = 100;
	 		}
	 		size = d.children ? 3.5 : Math.pow(orig_size, 2/10) || 1;
//	 		console.log(d.name+' has size '+size + ' and orig size '+orig_size);
	 		d.node_size = size
	 		return size;
	 		})
	 	.style('fill', function(d) {
	 		return ('url(#'+d.id+'-icon)');
	 	});
 	
 	// Add label/text to node
	vertex.append('text')
		.attr('id', function(d){ return 'node_'+d.id; })
		 .attr('class','node_label')
		 .attr('dx', function(d) {
			 return (d.node_size + 5);})
		 .attr('dy', '.35em')
		 .style('opacity', 0)
		 .text(function(d) {
			return (d.label ? d.label : d.name);})
//	.call(force.drag)
	 
	 d3.selectAll('.node_label').style('opacity', 0.5);

	// Node label showing up on mouse over
	node.append('title').text(function(d) {
		return d.name;
	});

	// test - is linked with
	root.links.forEach(function(d) {
		linkedByIndex[d.source.index + ',' + d.target.index] = 1;
	});

	// This functions look up whether a pair are neighbors
	function neighboring(a, b) {
		return linkedByIndex[a.index + ',' + b.index];
	}
	
	return graphModel;
}

function syntaxHighlight(json) {
  if (typeof json != 'string') {
       json = JSON.stringify(json, undefined, 2);
  }
  json = json.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
  return json.replace(/('(\\u[a-zA-Z0-9]{4}|\\[^u]|[^\\'])*'(\s*:)?|\b(true|false|null)\b|-?\d+(?:\.\d*)?(?:[eE][+\-]?\d+)?)/g, function (match) {
      var cls = 'number';
      if (/^'/.test(match)) {
          if (/:$/.test(match)) {
              cls = 'key';
          } else {
              cls = 'string';
          }
      } else if (/true|false/.test(match)) {
          cls = 'boolean';
      } else if (/null/.test(match)) {
          cls = 'null';
      }
      return '<span class="' + cls + '">' + match + '</span>';
  });
}

//this function displays a context menu on right click over a link
function linkpopup (d,i) {
	console.log('in linkpopup')
	$('g.link').contextMenu('cntxtMenu',
			{
    menuStyle: {
      listStyle: '',
      padding: '1px',
      margin: '0px',
      backgroundColor: '#fff',
      border: '1px solid #999',
      width: '300px'
    },
		itemStyle:
		{
			fontFamily : 'Arial',
			fontSize: '13px',
      width: '300px'
		},
		bindings:
		{
			'linkid': function(t) {
				linkinfo = JSON.stringify(t.__data__, null, '<br>');
				$('#linkInfoModal .modal-body').html(nodeinfo);
				$('#linkInfoModal').modal('show'); 
			}
		},
		onShowMenu: function(e, menu) {
			s = '<b>Node:</b> ' + d.name
			if (d.node_type) {
				s = s + '<br><b>Node type:</b> ' + d.node_type; 
			}
			if (d.protocols) {
				s = s + '<br><b>Protocols:</b> ' + d.protocols; 
			}
			if (d.s_cnt) {
				s = s + '<br><b>Packets sent:</b> ' + d.s_cnt; 
			}
			if (d.r_cnt) {
				s = s + '<br><b>Packets received:</b> ' + d.r_cnt; 
			}
			if (d.sourceof) {
				s = s + '<br><b>Destinations:</b> ' + d.sourceof.length; 
			}
			if (d.targetof) {
				s = s + '<br><b>Sources:</b> ' + d.targetof.length; 
			}
				
			$('#linkid', menu).html(s);
			return menu;
		}
			});
}
			
//this function displays a context menu on right click
function nodepopup (d,i) {
	$('g.node').contextMenu('cntxtMenu',
			{
    menuStyle: {
      listStyle: '',
      padding: '1px',
      margin: '0px',
      backgroundColor: '#fff',
      border: '1px solid #999',
      width: '300px'
    },
		itemStyle:
		{
			fontFamily : 'Arial',
			fontSize: '13px',
      width: '300px'
		},
		bindings:
		{
			'nodeid': function(t) {
				ctrlScope.$apply(function() {
					ctrlScope.selectedNode = t.__data__.name;
				});
				document.getElementById('nodeId').value = t.__data__.id;
				newTab = window.open('/nodeDetails?selectedNode="'+ctrlScope.selectedNode+'"');
				newTab.selectedNode=ctrlScope.selectedNode;
				newTab.nodeLinks=ctrlScope.nodeLinks;
			},
			'action': function(t) {
				alert('Perform action X -to be determined- on node '+t.__data__.name);
			},
			'rename': function(t) {
				document.getElementById('nodeId').value = t.__data__.id;
				document.getElementById('currentName').value = t.__data__.name;
				document.getElementById('currentName').readOnly = true;
				document.getElementById('newName').autofocus = true;
				document.getElementById('newName').value = '';
				document.getElementById('nodeNameModalTitle').innerHTML = 'Rename '+t.__data__.name;
				$('#nodeNameModal').modal('show'); 
			},
			'changetype': function(t) {
				document.getElementById('nodeId').value = t.__data__.id;
				document.getElementById('nodeName').value = t.__data__.name;
				document.getElementById('nodeTypeModalLabel').innerHTML = 'Change type of '+ t.__data__.name;
				$('#nodeTypeModal').modal('show'); 
			},
			'delete': function(t) {
				$('g.node').remove();
			}
		},
		onShowMenu: function(e, menu) {
			s = '<b>Node:</b> ' + d.name
			if (d.uids) {
				s = s + '<br><b>UIDs:</b> ' + d.uids; 
			}
			if (d.node_type) {
				s = s + '<br><b>Node type:</b> ' + d.node_type; 
			}
			if (d.protocols) {
				s = s + '<br><b>Protocols:</b> ' + d.protocols; 
			}
			if (d.s_cnt) {
				s = s + '<br><b>Packets sent:</b> ' + d.s_cnt; 
			}
			if (d.r_cnt) {
				s = s + '<br><b>Packets received:</b> ' + d.r_cnt; 
			}
			if (d.sourceof) {
				s = s + '<br><b>Destinations:</b> ' + d.sourceof.length; 
			}
			if (d.targetof) {
				s = s + '<br><b>Sources:</b> ' + d.targetof.length; 
			}
				
			$('#nodeid', menu).html(s);
			return menu;
		}
			});
}


/*
 * Zoom
 */
function zoomer() {
	canvas.attr('transform', 'translate(' + d3.event.translate + ')' + ' scale('
			+ d3.event.scale + ')');
}

/*
 * Drag, pin & release
 */
function dragstarted(d) {
	d3.event.sourceEvent.stopPropagation();
	d3.select(this).classed('dragging', true);
	force.stop();
}

function dragged(d) {

//	d.px = Math.min(d.x + d3.event.x, width);
//	d.py = Math.min(d.y + d3.event.y, height);
//	d.x = d.px;
//	d.y = d.py;
	
//	d.px += d3.event.dx;
//	d.py += d3.event.dy;
//	
//  if (d.py > height) d.py = height;
//  if (d.py < 50) d.py = 50;
//
//  if (d.px > width) d.px = width;
//  if (d.px < 50) d.px = 50;
//
//  d.x = d.px;
//  d.y = d.py;
  

	d.px += d3.event.dx;
	d.py += d3.event.dy;
	d.x += d3.event.dx;
	d.y += d3.event.dy;
	node.attr('transform', function(d) {
		return 'translate(' + [ d.x, d.y ] + ')';
	});

}

//function dragmove(d) {
//	radius = d.node_size;
//    var dx = function(d) {return Math.max(radius, Math.min(width - radius, d.x))}
//    var dy = function(d) {return Math.max(radius, Math.min(width - radius, d.y))}
//    d.px = Math.min(d3.event.x,200);
//    d.py = Math.min(d3.event.y, 200);
//    d.x = Math.min(d3.event.x, 200);
//    d.y = Math.min(d3.event.y, 200);
//}

function dragended(d) {
	d3.select(this).classed('dragging', false);
	d.fixed = true; // set the node to fixed so the force doesn't include the node
									// in its auto positioning stuf
	force.resume();
}
function releasenode(d) {
	d.fixed = false; // set the node to fixed so the force doesn't include the
										// node in its auto positioning stuff
	force.resume();
}

function clearsearch() {
	$('#searchinput').autocomplete('close').val('');
	if (currently_selected_node) {
		currently_selected_node.each(mouseouted);
	}
};
function tog(v) {
	return v ? 'addClass' : 'removeClass';
};
