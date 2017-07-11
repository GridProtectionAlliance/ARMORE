
angular.module('d3', []).factory('d3Service',
		[ '$document', '$q', '$rootScope', function($document, $q, $rootScope) {
			var d = $q.defer();
			function onScriptLoad() {
				// Load client in the browser
				$rootScope.$apply(function() {
					d.resolve(window.d3);
				});
			}
			// Create a script tag with d3 as the source
			// and call our onScriptLoad callback when it
			// has been loaded
			var scriptTag = $document[0].createElement('script');
			scriptTag.type = 'text/javascript';
			scriptTag.async = true;
			scriptTag.src = '/static/js/d3.v3.min.js';
			scriptTag.onreadystatechange = function() {
				if (this.readyState == 'complete')
					onScriptLoad();
			}
			scriptTag.onload = onScriptLoad;

			var s = $document[0].getElementsByTagName('body')[0];
			s.appendChild(scriptTag);

			return {
				d3 : function() {
					return d.promise;
				}
			};
		} ]);

// The main application
var vizApp = angular.module('Visualization', [ 'd3', 'ngRoute', 'ngSanitize', 'angularjs-dropdown-multiselect' ]);

// Add a 'debug' filter to check whether an angular-computed string is empty
vizApp.filter('debug', function() {
  return function(input) {
    if (input === '') return 'empty string';
    return input ? input : ('' + input);
  };
});
vizApp.filter('trustedHtml', function($sce) { return $sce.trustAsHtml; });

vizApp.filter('namedValues', 
		function() {
		return function(d) {
			return getNamedValues(d).sort().join("<br>");
		};
	})
	
vizApp.filter('linksForNode', 
		function() {
		return function(links, selNode, searchWord) {
			filteredLinks = []
			for (i = 0; i< links.length; i++) {
				link = links[i];
				if (link.source_name == selNode | link.target_name == selNode) {
					if (!searchWord)
						filteredLinks.push(link);
					else if (link.proto_func_target.toUpperCase().includes(searchWord.toUpperCase()) ||
							link.source_name.toUpperCase().includes(searchWord.toUpperCase()) ||
							link.target_name.toUpperCase().includes(searchWord.toUpperCase())) {
						filteredLinks.push(link);
					}
				}
			}
			return filteredLinks;
		};
	})

// Define all directives and controllers
vizApp.factory('SharedData', SharedData);
vizApp.controller('CollectionController', ['$scope', '$sce', '$http', 'SharedData', CollectionController]);
vizApp.controller('NodeController', ['$scope', '$sce', '$http', 'SharedData', NodeController]);
vizApp.controller('NodeDetailsController', ['$scope', NodeDetailsController]);
vizApp.directive('drawHistoAnomalies',	['d3Service', 'SharedData', drawHistoAnomalies]);
vizApp.directive('drawGraph',	['d3Service', 'SharedData', drawGraph]);
vizApp.directive('currentTime', [ '$interval', 'dateFilter', currentTime]);

vizApp.directive('contextMenu', function(){
  return {
      restrict: 'A',
      scope: {
          target: '@'
      },
      link: function(scope, element){
          console.log('contextmenu: '+scope);
      }
  };
});

// Draw histogram of anomalies
function drawHistoAnomalies (d3Service, SharedData) {
	return {
		restrict : 'EA',
		transclude : true,
		scope : '&',
		link : function($scope, element, attrs) {
      $scope.model = SharedData.sharedObject;
			// Query list of anomalies
			console.log("retrieving anomalies from database...");
			d3.json("/stats_anomalies",
					function(error, jdocs) {
					if (error)
						throw error;
					if (jdocs.length > 0) {
						$scope.model.anomalies = jdocs;

						for (i = 0; i< $scope.model.anomalies.length; i++) {
							for (j = 0; j< $scope.model.anomalies[i].anomalies.length; j++) {
								anom = $scope.model.anomalies[i].anomalies[j]
								sr = anom['sender-receiver'].split("-");
								anom['sender'] = sr[0];
								anom['receiver'] = sr[1];
							}
						}
					  $scope.model.anoSelected  = jdocs[jdocs.length-1]['created_at'];  // the
																																							// bar/created_at
																																							// selected
																																							// in
																																							// the
																																							// histogram
					  $scope.anoSortType     = 'desc'; // set the default sort type
					  $scope.anoSortReverse  = false;  // set the default sort order
					  $scope.anoSearch  = '';  // set the default search term
						buildAnomalyHistogram($scope.model);
						$scope.$watch("model.anoSelected", function(){
							console.log('Selected anomalies for date '+$scope.model.anoSelected);
							$scope.model.selectedDate = $scope.model.anoSelected;
						})
						node = d3.select('#graph_svg').selectAll('link')
				    .attr("class", "mouna")

						node
						.append('defs')
							.append('anomalies')
								.attr('count', function(d) {console.log('here')})
					
				}
					
					function buildAnomalyHistogram(model_ano){
						console.log('Parse anomalies...');

						console.log(JSON.stringify(model_ano.anomalies));
					// Set the dimensions of the canvas
						var margin = {top: 20, right: 20, bottom: 70, left: 40},
						    width = 600 - margin.left - margin.right,
						    height = 300 - margin.top - margin.bottom;
						
						var tip = d3.tip()
					  .attr('class', 'd3-tip')
					  .offset([-10, 0])
					  .html(function(d) { return d; })
					  
						// Set the ranges
						var x = d3.scale.ordinal().rangeRoundBands([0, width], .05);

						var y = d3.scale.linear().range([height, 0]);

						// Define the axis
						var xAxis = d3.svg.axis()
						    .scale(x)
						    .orient("bottom")


						var yAxis = d3.svg.axis()
						    .scale(y)
						    .orient("left")
						    .ticks(10);

						// Add the SVG element
						var svg = d3.select("#histoContent").append("svg")
						    .attr("width", width + margin.left + margin.right)
						    .attr("height", height + margin.top + margin.bottom)
						  .append("g")
						    .attr("transform", 
						          "translate(" + margin.left + "," + margin.top + ")");

						svg.call(tip);
						
					  // Scale the range of the data
					  x.domain(model_ano.anomalies.map(function(d) { return d.created_at; }));
					  y.domain([0, d3.max(model_ano.anomalies, function(d) { return d.anomalies.length; })]);

					  // add axis
					  svg.append("g")
					      .attr("class", "x axis")
					      .attr("transform", "translate(0," + height + ")")
					      .call(xAxis)
					    .selectAll("text")
					      .style("text-anchor", "end")
					      .attr("dx", "-.8em")
					      .attr("dy", "-.55em")
					      .attr("transform", "rotate(-90)" );

					  svg.append("g")
					      .attr("class", "y axis")
					      .call(yAxis)
					    .append("text")
					      .attr("transform", "rotate(-90)")
					      .attr("y", 5)
					      .attr("dy", ".71em")
					      .style("text-anchor", "end")
					      .text("Anomalies");

					  // Add bar chart
					  svg.selectAll("bar")
					      .data(model_ano.anomalies)
					    .enter().append("rect")
					      .attr("class", "bar")
					      .attr("x", function(d) { return x(d.created_at); })
					      .attr("width", x.rangeBand())
					      .attr("y", function(d) { return y(d.anomalies.length); })
					      .attr("height", function(d) { return height - y(d.anomalies.length); })
					  		.on('mouseover', function(d, i) {
// console.log(JSON.stringify(d));
					  			tip_msg = d.anomalies.length;
					  			if (d.anomalies.length > 1) {
					  				tip_msg += ' anomalies'
						  		}
						  		else {
										tip_msg += ' anomaly'
						  		}
					  		  tip.show(tip_msg, this)
					  		})
					      .on('mouseout', tip.hide)
					      .on('click', function(d, i) {
					      	d3.selectAll("rect.bar").style("fill","steelblue");
					      	d3.select(this).style("fill","brown");
					  			$scope.$apply(function() {
					  		    $scope.model.anoSelected = d.created_at;
					  			});
					  		});
					}
			});
		}
	}
}

// Draw node graph - from stats_cols
function drawGraph (d3Service, SharedData) {
					return {
						restrict : 'EA',
						transclude : true,
						scope : false, // use parent scope
						link : function($scope, element, attrs) {
							$scope.showLabels = true;
				      $scope.model = SharedData.sharedObject;

							// -- Function to update graph and tree based on user selection in
							// timestamps list
							$scope.$watch('model.selectedDate', function() {
								d3Service.d3()
								.then(
										function(d3) {
											d3.select("#graphContent").selectAll("*").remove();
											d3.select("#treeContent").selectAll("*").remove();
											
											// Initialize canvas
											// - Graph canvas
											canvas = d3.select("#graphContent").append("svg")
											.attr("id","graph_svg")
											// .attr("width", width).attr("height", height)
											.attr("viewBox", "0 0 " + width + " " + height)
											// .attr("pointer-events", "all")
											.call(
													d3.behavior.zoom().scaleExtent([ 1, 5 ]).on("zoom",
															zoomer)).on("dblclick.zoom", null).append('svg:g');
											  
												// Query json graph data from database
												console.log("retrieving json logs from database...");
												console.log("/stats_jlogs?collectionTime="+$scope.model.selectedDate);
												d3.json("/stats_jlogs?collectionTime="+$scope.model.selectedDate,
														function(error, jdocs) {
													if (error)
														throw error;
													if (jdocs.length > 0) {
														data_col = jdocs[0]['col_log']
														data_count = jdocs[0]['count_log']
														
														$scope.graphModel = buildGraph($scope, data_col);
														updateGraphFilters($scope);
														updateLinksSummary($scope);
														// - Tree canvas
														if (document.getElementById("treeContent")) {
															t_svg = d3.select("#treeContent")
															.append("svg")
															.attr("id","tree_svg")
															.attr("width", t_width + margin.right + margin.left)
															.attr("height",
																			t_height + margin.top + margin.bottom).append(
																			"g")
															.attr("transform",
																			"translate(" + margin.left + "," + margin.top
																					+ ")");														
															buildTree(data_count);
													}
													}
												});
// var ordinal = d3.scale.ordinal()
// .domain(["a", "b", "c", "d", "e"])
// .range([ "rgb(153, 107, 195)", "rgb(56, 106, 197)", "rgb(93, 199, 76)",
// "rgb(223, 199, 31)", "rgb(234, 118, 47)"]);

// var svg = d3.select("svg");

// canvas.append("g")
// .attr("class", "legendOrdinal")
// .attr("transform", "translate(20,20)");
//
// var legendOrdinal = d3.legend.color()
// //d3 symbol creates a path-string, for example
// //"M0,-8.059274488676564L9.306048591020996,
// //8.059274488676564 -9.306048591020996,8.059274488676564Z"
// .shape("path", d3.svg.symbol().type("triangle-up").size(150)())
// .shapePadding(10)
// .scale(ordinal);
//
// canvas.select(".legendOrdinal")
// .call(legendOrdinal);
										}
									);
							});
						}
					}
				};

// Update filter dropdown with the attributes found in the json graph
function updateGraphFilters ($scope) {
	if ($scope.graphModel == null) {
		return;
	}
	$scope.model.changes = $scope.graphModel['changes'];
	if ($scope.model.changes) {
		$scope.model.comparison_mode = true;
	}
	if ($scope.graphModel['changes']) {
		alert(JSON.stringify($scope.graphModel['changes'], null, '\n'));
	}
	$scope.filterOptions = [{'id':1, 'label':'labels', 'type': 'display'}];
	$scope.filterOptionsByIndex = {}
	$scope.filterListModel = [{'id':1, 'label':'labels', 'type':'title'}];
	idx = $scope.filterOptions.length
	for (var key in $scope.graphModel) {
	  if ($scope.graphModel.hasOwnProperty(key) && Array.isArray($scope.graphModel[key])) {
	  	var sortedValues = $scope.graphModel[key].sort()
			for (j = 0; j < sortedValues.length; j++) {
				idx += 1;
				$scope.filterOptions.push({'id':idx, 'label':$scope.graphModel[key][j], 'type': key});
				$scope.filterOptionsByIndex[idx] = {'label':$scope.graphModel[key][j], 'type': key}
				$scope.filterListModel.push({'id':idx, 'label':$scope.graphModel[key][j], 'type': key});
			}
	  }
	}
// console.log('$scope.filterOptions = '+JSON.stringify($scope.filterOptions));
}

//Update links with the attributes found in the json graph
function updateLinksSummary ($scope, $sce) {
if ($scope.graphModel == null) {
	return;
}

// Sort links by source name first and target second
function sortLinks(ob1,ob2) {
  if (ob1.source_name > ob2.source_name) {
      return 1;
  } else if (ob1.source_name < ob2.source_name) { 
      return -1;
  }

  // Else go to the 2nd item
  if (ob1.target_name < ob2.target_name) { 
      return -1;
  } else if (ob1.target_name > ob2.target_name) {
      return 1
  } else { // nothing to split them
      return 0;
  }
}

sortedLinks = $scope.graphModel['links'].sort(sortLinks);
for (var i = 0; i < sortedLinks.length; i++) {
	lnk = $scope.graphModel['links'][i];
	
//	console.log('link '+JSON.stringify(dictValues(lnk['target_name'])));
	if (lnk['target_name'] == '-' | lnk['source_name'] == '-') {
		continue;
	}
	
	$scope.baselineLinks.push({'sender':lnk['source_name'],
		'receiver':lnk['target_name'],
		'target': getNamedValues(lnk['TARGET']).join("<br>"),
		'protocol': getNamedValues(lnk['PROTOCOL']).join("<br>"),
		'function': getNamedValues(lnk['FUNCTION']).join("<br>"),
		'uid': getNamedValues(lnk['uid']).join("<br>"),
		'proto_func_target': lnk['proto_func_target'],
		'navg': getNamedValues(lnk['navg']).join("<br>"),
		'nstd': getNamedValues(lnk['nstd']).join("<br>"),
		'bavg': getNamedValues(lnk['bavg']).join("<br>"),
		'bstd': getNamedValues(lnk['bstd']).join("<br>"),
		'ravg': getNamedValues(lnk['ravg']).join("<br>"),
		'rstd': getNamedValues(lnk['rstd']).join("<br>"),
		'davg': getNamedValues(lnk['davg']).join("<br>"),
		'dstd': getNamedValues(lnk['dstd']).join("<br>")});
	}
}

function getNamedValues (d) {
	var named_vals = [];
	if (d) {
		for (var i = 0; i < dictValues(d).length; i++) {
			named_vals.push(d[i]['name']);
		}
	}
	return named_vals;
}

// Function showing the current time
function currentTime ($interval, dateFilter) {
			function link(scope, element, attrs) {
				var format = 'M/d/yy h:mm:ss a';
				var timeoutId;

				function updateTime() {
					element.text(dateFilter(new Date(), format));
				}

				element.on('$destroy', function() {
					$interval.cancel(timeoutId);
				});

				// start the UI update process; save the timeoutId for canceling
				timeoutId = $interval(function() {
					updateTime(); // update DOM
				}, 1000);
			}

			return {
				link : link
			};
		};

// Data shared by controllers
function SharedData () {
  return {
    sharedObject: {
      selectedDate: 'n/a',
      baseline: 'baseline',
      dateList:'',
      compare2Date: 'n/a'
    }
  };
};

Object.toparams = function ObjecttoParams(obj) {
  var p = [];
  for (var key in obj) {
      p.push(key + '=' + encodeURIComponent(obj[key]));
  }
  return p.join('&');
};


function NodeDetailsController ($scope) {
	$scope.nodeLinks = window.nodeLinks;
	$scope.selectedNode = window.selectedNode;
  $scope.linkSortType     = 'source_name'; // set the default sort type
  $scope.linkSortReverse  = false;  // set the default sort order
}

function NodeController ($scope, $sce, $http, SharedData) {
	$scope.model = SharedData.sharedObject;
	$scope.nodeTypeImage = nodeTypeImage;

//	$scope.selectedNode = null;
	// Function used to rename a node in the graph
	$scope.graphRenameNode = function graphRenameNode () {
		// Update node label in the graph and in DB
		oldName = document.getElementById("currentName").value;
		newName = document.getElementById("newName").value;
		if (newName != oldName) {
			var xsrf = $.param({
				'timestamp' : $scope.model.selectedDate,
				'nodeName' : oldName,
				'nodeLabel' : newName});
			Object.toparams = 
			$http({
			    method: 'POST',
			    url: '/updateNode',
			    data: xsrf,
			    headers: {'Content-Type': 'application/x-www-form-urlencoded'}
			});
		}
		// Update displayed node label
		$scope.selectedNode = d3.select('#node_'+document.getElementById('nodeId').value)
		if (newName){
			$scope.selectedNode.text(newName);
		}
		else {
			$scope.selectedNode.text(oldName);
		}
		// Hide modal
		$('#nodeNameModal').modal('hide'); 
	};
	
	// Function used to change node type and icon
	$scope.graphChangeNodeType = function graphChangeNodeType () {
		var selectedType = $('#nodeTypeSelector input:radio:checked').val()
		var ndname = document.getElementById("nodeName").value;
		// Update node_type in database
		var xsrf = $.param({
			'timestamp' : $scope.model.selectedDate,
			'nodeName' : ndname,
			'nodeType' : selectedType});
		Object.toparams = 
		$http({
		    method: 'POST',
		    url: '/updateNode',
		    data: xsrf,
		    headers: {'Content-Type': 'application/x-www-form-urlencoded'}
		});
			
		// Update displayed node
		selectedNodeImg = d3.select('#node_img_'+document.getElementById('nodeId').value)
		selectedNodeImg.attr("xlink:href", imageLocation+nodeTypeImage[selectedType]);

		// Hide modal
		$('#nodeTypeModal').modal('hide'); 		
	}
};

function CollectionController($scope, $sce, $http, SharedData) {
	$scope.model = SharedData.sharedObject;
	$scope.nodeName="N/A";
	$scope.model.comparison_mode = false;
	$scope.baselineLinks = [];
	$scope.nodeLinks = [];
	$scope.selectedNode = null;
	$scope.baselineSortType = 'sender';
	
  $scope.init = function(selDate) {
  	$scope.model.selectedDate = selDate;
  }
	$('#viewSelector button.btn').click(function() {
	  $(this).addClass('active').siblings().removeClass('active');
	  $(this).addClass('active').siblings().removeClass('btn-success');
	  $(this).addClass('btn-success');
	  
	  if ($(this).html() == 'Graph') {
	  	scrollToElement(document.getElementById("graph_box"), undefined, undefined, undefined, undefined);// ()
																																																				// =>
																																																				// alert('done'))
	  }
	  else if ($(this).html() == 'Tree') {
	  	scrollToElement(document.getElementById("tree_box"), undefined, undefined, undefined, undefined);// ()
																																																				// =>
																																																				// alert('done'))
	  }
	  else {
	  	document.getElementById("armore").scrollIntoView();	  	
	  }
	});

	// Dates list box 1
	$scope.dateListData = [];
	$scope.dateListModel = [];
	$scope.dateListSettings = {
			displayProp: 'label',
			dynamicTitle : true, // use true to show selected date on button
      scrollableHeight: '300px',
      scrollable: true,
      closeOnSelect : true,
      enableSearch: true,
      idProp: 'label',
      showCheckAll : false,
      showUncheckAll : false,
      selectionLimit: 1,
      smartButtonMaxItems: 1,
      smartButtonTextConverter: function(itemText, originalItem) {
        return itemText;
    }
  };
	$scope.dateListCustomTexts = {
			buttonDefaultText: 'Select date...'	
	};
	$scope.dateListEvents = {
			onItemSelect : function(item) {
	  		$scope.dateListModel = [{'id':item.id}];
		  	if (item && item.id !=  $scope.model.selectedDate) {
		      $scope.model.selectedDate = item.id;
		  	};
			}
	};
	

	// Dates list box 2
	$scope.compare2DateList = [];
	$scope.compare2DateModel = [];
	$scope.compare2DateListSettings = {
			displayProp: 'label',
			dynamicTitle : true, // use true to show selected date on button
      scrollableHeight: '300px',
      scrollable: true,
      closeOnSelect : true,
      enableSearch: true,
      idProp: 'label',
      showCheckAll : false,
      showUncheckAll : false,
      selectionLimit: 1,
      smartButtonMaxItems: 1,
      smartButtonTextConverter: function(itemText, originalItem) {
        return itemText;
    }
  };
	$scope.compare2DateListCustomTexts = {
			buttonDefaultText: 'Compare to...'	
	};
	$scope.compare2DateListEvents = {
			onItemSelect : function(item) {
	  		$scope.compare2DateModel = [{'id':item.id}];
		  	if (item && item.id !=  $scope.model.selectedDate) {
		      $scope.model.selectedCompare2Date = item.id;
		      // Query json graph data from database
		      cmd = "/compare?timestamp1="+
					$scope.model.selectedDate+"&timestamp2="+
					$scope.model.selectedCompare2Date
					console.log("Comparing json logs "+
							$scope.model.selectedDate+"-"+
							$scope.model.selectedCompare2Date);
					console.log("Sending comparison request: "+cmd);
					d3.json(cmd,
							function(error, jdocs) {
						if (error)
							throw error;
						if (jdocs.length > 0) {
							data_col = jdocs[0]['col_log']
							data_count = jdocs[0]['count_log']
							
							$scope.graphModel = buildGraph($scope, data_col);
							updateGraphFilters($scope);
							
							buildTree(data_count);
						}
						else {
							console.log("got back: "+ JSON.stringify(jdocs));
						}
					});
		  	};
			}
	};
		
	console.log("retrieving json logs creation times from database...");
	$http.get('/stats_jlogs_times').then(function(data) {
		$scope.dates = data['data'];
		if ($scope.model.selectedDate == "") {
			$scope.model.selectedDate = $scope.dates[0];
		}
		$scope.dateListModel = [{'id':$scope.dates[0]}];
		for (i = 0; i < $scope.dates.length; i++) {
			$scope.dateListData.push({'id':i+1, 'label':$scope.dates[i]});
			$scope.compare2DateList.push({'id':i+1, 'label':$scope.dates[i]});
		}
	});
	
	// Filter list box
	$scope.filterOptions = [{'id':1, 'label':'labels', 'type': 'display'}];
	$scope.filterOptionsByIndex = {}
	$scope.filterListModel = [{'id':1}];
	$scope.filterListSettings = {
			displayProp: 'label',
			dynamicTitle : false,
      scrollableHeight: '300px',
      scrollable: true,
      showCheckAll : true,
      showUncheckAll : true,
      groupByTextProvider: function(groupValue) { 
      	return groupValue;
      }
  };
	$scope.filterListCustomTexts = {
			buttonDefaultText: 'Show'			
	};
	$scope.filterListEvents = {
			onItemSelect : function(item) {
				filterSelectItem($scope, item);
			},

			onItemDeselect : function(item) {
				filterDeselectItem($scope, item);
			},
			
			onDeselectAll : function() {
				filterDeselectAll($scope);
			},
			
			onSelectAll : function() {
				filterSelectAll($scope);
			}
	};
	
	function filterLinks(item, selected) {
		var link = d3.selectAll(".link")
		link.style("visibility", function(lnk) {
			visibility = "visible";
			if (lnk.visible == false) {
				visibility = "hidden";
			}

			link_protos = dictValues(lnk.PROTOCOL[0]);
			link_funcs = dictValues(lnk.FUNCTION[0]);
// console.log("\nlink
// "+lnk.id+":"+JSON.stringify(lnk.source.name)+">"+JSON.stringify(lnk.target.name)+",
// "+lnk.visible+" has protos/funcs "+JSON.stringify(link_protos)+",
// "+JSON.stringify(link_funcs)+" comes as visible: "+lnk.visible);

			// Selected filter protocols
			if (item.type == 'protocols') {
				if (isInArray(item.label, link_protos)) {
					if (selected == false) {
						visibility = "hidden";
						lnk.visible = false;
					}
					else {
						visibility = "visible";
						lnk.visible = true;
					}
				}
			}
			if (item.type == 'functions') {
				if (isInArray(item.label, link_funcs)) {
					if (selected == false) {
						visibility = "hidden";
						lnk.visible = false;
					}
					else {
						visibility = "visible";
						lnk.visible = true;
					}
				}
			}
			if (visibility == "hidden") {
				lnk.visible = false;
			}
			else {
				lnk.visible = true;
			}
			return visibility;
		});
	};

	function filterSelectItem(scope, item) {
		// Action taken when user make a selection in the filter list box
		if (item) {
			if (item.id == 1) {
		    scope.showLabels = true;
		    graphSwitchLabelDisplay(scope.showLabels);
			}
			else {
				// Show matching links
				f = scope.filterOptionsByIndex[item.id];
				filterLinks(f, true);
				// If filter is protocol, automatically check all corresponding
				// functions and targets options in filter list box
				if (f.type == "protocols") {
					filtered_proto = f.label
					for (j = 0; j < scope.filterOptions.length; j++) {
						if (scope.filterOptions[j].label.startsWith(filtered_proto+':')) {
	  					scope.filterListModel.push({'id':$scope.filterOptions[j].id, 
	  						'label':scope.filterOptions[j].label,
	  						'type':'protocols'}); 
	  					}
						}
					}
				else {
					filtered_proto = f.label.split(':')[0];
					proto_selected = false;
					for (j = 0; j < scope.filterListModel.length; j++) {
						if (scope.filterListModel[j].label == filtered_proto) {
							proto_selected = true;
							break
	  					}
						}
					if (!proto_selected) {
						for (j = 0; j < scope.filterOptions.length; j++) {
							if (scope.filterOptions[j].label == filtered_proto) {
		  					scope.filterListModel.push({'id':$scope.filterOptions[j].id, 
		  						'label':scope.filterOptions[j].label,
		  						'type':'protocols'}); 
		  					}
							}
						}
					}
				};
			};
		}
	
	function filterDeselectItem(scope, item) {
		// Action taken when user deselect an option in the filter list box
		if (item && item.id == 1) {
		    scope.showLabels = false;
		    graphSwitchLabelDisplay(scope.showLabels);
	  	}
			else {
				// Hide matching links
				f = scope.filterOptionsByIndex[item.id];
				filterLinks(f, false);
				// If filter is protocol, automatically deselect all corresponding
				// functions and targets options in filter list box
				if (f.type == "protocols") {
					filtered_proto = f.label
					for (j = 0; j < scope.filterOptions.length; j++) {
						idx = -1;
						if (scope.filterOptions[j].label.startsWith(filtered_proto+':')) {
							for (var k=0; k < scope.filterListModel.length; k++) {
								if (scope.filterListModel[k]['id'] == scope.filterOptions[j].id) {
									idx = k;
									break;
								}
							}
	  					scope.filterListModel.splice(idx, 1);
						}
					}
				}
				else if (f.type == "functions" || f.type == "targets") {
// console.log(JSON.stringify(scope.filterListModel));
					filtered_proto = f.label.split(':')[0];
					uncheck_proto = true;
					for (var k=0; k < scope.filterListModel.length; k++) {
						if (scope.filterListModel[k]['type'] == f.type && scope.filterListModel[k]['label'].startsWith(filtered_proto + ':')) {
							uncheck_proto = false;
							console.log('found checked option '+scope.filterListModel[k]['type']+' '+scope.filterListModel[k]['label']);
							break;
						}
					}
					if (uncheck_proto == true) {
						matched = [];
						notdone = true;
						console.log('filterListModel: '+ JSON.stringify(scope.filterListModel));
						while (notdone) {
							idx = -1;
							for (j = 0; j < scope.filterListModel.length; j++) {
								if (scope.filterListModel[j].label.startsWith(filtered_proto)) {
									idx = j;
									break
								}
							}
							if (idx != -1) {
								scope.filterListModel.splice(idx, 1);
							}
							else {
								notdone = false;
							}	
						}
					}
				}
			};
		}
	
	function filterDeselectAll(scope) {
		// Action taken when user deselect all options in the filter list box
		for (i = 0; i < scope.filterOptions.length; i++) {
			filterDeselectItem(scope, scope.filterOptions[i]);
		}
	}
	
	function filterSelectAll(scope) {
		// Action taken when user selects all options in the filter list box
		for (i = 0; i < scope.filterOptions.length; i++) {
			filterSelectItem(scope, scope.filterOptions[i]);
		}
	}
};

function scrollToElement(element, duration = 400, delay = 0, easing = 'cubic-in-out', endCallback = () => {}) {
  var offsetTop = window.pageYOffset || document.documentElement.scrollTop
  d3.transition()
    .each("end", endCallback)
    .delay(delay)
    .duration(duration)
    .ease(easing)
    .tween("scroll", (offset => () => {
      var i = d3.interpolateNumber(offsetTop, offset);
      return t => scrollTo(0, i(t))
    })(offsetTop + element.getBoundingClientRect().top));
}

// -- Utility Methods
function dictValues(dict) {
	if (typeof(dict) === 'string')
		return [];
	return $.map(dict, function(value, key) { return value });
}

$(document).ready(function(){
  $(window).scroll(function () {
         if ($(this).scrollTop() > 50) {
             $('#back-to-top').fadeIn();
         } else {
             $('#back-to-top').fadeOut();
         }
     });
     // scroll body to 0px on click
     $('#back-to-top').click(function () {
         $('#back-to-top').tooltip('hide');
         $('body,html').animate({
             scrollTop: 0
         }, 800);
         return false;
     });
});