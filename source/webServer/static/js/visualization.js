
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

var vizApp = angular.module('Visualization', [ 'd3', 'ngRoute', 'angularjs-dropdown-multiselect' ]);

vizApp.factory('SharedData', SharedData);
vizApp.controller('CollectionController', ['$scope', '$http', 'SharedData', CollectionController]);
vizApp.controller('NodeController', ['$scope', '$http', 'SharedData', NodeController]);
vizApp.directive('drawHisto',	['d3Service', 'SharedData', drawHisto]);
vizApp.directive('drawGraph',	['d3Service', 'SharedData', drawGraph]);
vizApp.directive('currentTime', [ '$interval', 'dateFilter', currentTime]);

// Draw histogram of changes
function drawHisto (d3Service, SharedData) {
	return {
		restrict : 'EA',
		transclude : true,
		scope : false,
		link : function($scope, element, attrs) {
      $scope.model = SharedData.sharedObject;
			// Query list of anomalies
			console.log("retrieving anomalies from database...");
			console.log("/stats_anomalies");
			d3.json("/stats_anomalies",
					function(error, jdocs) {
				if (error)
					throw error;
				if (jdocs.length > 0) {
					$scope.anomalyModel = buildAnomalyHistogram(jdocs);
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
						scope : false,
						link : function($scope, element, attrs) {
							$scope.showLabels = true;
				      $scope.model = SharedData.sharedObject;
				      
							//-- Function to update graph and tree based on user selection in timestamps list
							$scope.$watch('model.selectedDate', function() {
								d3Service.d3()
								.then(
										function(d3) {
											d3.select("#graphContent").selectAll("*").remove();
											d3.select("#treeContent").selectAll("*").remove();
											
											// Initialize canvas
											//- Graph canvas
											canvas = d3.select("#graphContent").append("svg")
											.attr("id","graph_svg")
											// .attr("width", width).attr("height", height)
											.attr("viewBox", "0 0 " + width + " " + height)
											// .attr("pointer-events", "all")
											.call(
													d3.behavior.zoom().scaleExtent([ 1, 5 ]).on("zoom",
															zoomer)).on("dblclick.zoom", null).append('svg:g');
											  
											//- Tree canvas
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
													
													$scope.graphModel = buildGraph(data_col);
													updateGraphFilters($scope);
													
													buildTree(data_count);
												}
											});
//											var ordinal = d3.scale.ordinal()
//										  .domain(["a", "b", "c", "d", "e"])
//										  .range([ "rgb(153, 107, 195)", "rgb(56, 106, 197)", "rgb(93, 199, 76)", "rgb(223, 199, 31)", "rgb(234, 118, 47)"]);

//										var svg = d3.select("svg");

//										canvas.append("g")
//										  .attr("class", "legendOrdinal")
//										  .attr("transform", "translate(20,20)");
//
//										var legendOrdinal = d3.legend.color()
//										  //d3 symbol creates a path-string, for example
//										  //"M0,-8.059274488676564L9.306048591020996,
//										  //8.059274488676564 -9.306048591020996,8.059274488676564Z"
//										  .shape("path", d3.svg.symbol().type("triangle-up").size(150)())
//										  .shapePadding(10)
//										  .scale(ordinal);
//
//										canvas.select(".legendOrdinal")
//										  .call(legendOrdinal);
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
//	console.log('$scope.filterOptions = '+JSON.stringify($scope.filterOptions));
}

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

function SharedData () {
  return {
    sharedObject: {
      selectedDate: 'n/a',
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

function NodeController ($scope, $http, SharedData) {
	$scope.model = SharedData.sharedObject;
	$scope.nodeTypeImage = nodeTypeImage;
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
		selectedNode = d3.select('#node_'+document.getElementById('nodeId').value)
		if (newName){
			selectedNode.text(newName);
		}
		else {
			selectedNode.text(oldName);
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

function CollectionController($scope, $http, SharedData) {
	$scope.model = SharedData.sharedObject;
	$scope.nodeName="N/A";
	$scope.model.comparison_mode = false;

	$('#viewSelector button.btn').click(function() {
	  $(this).addClass('active').siblings().removeClass('active');
	  $(this).addClass('active').siblings().removeClass('btn-success');
	  $(this).addClass('btn-success');
	  
	  if ($(this).html() == 'Graph') {
	  	scrollToElement(document.getElementById("graph_box"), undefined, undefined, undefined, undefined);//() => alert('done'))
	  }
	  else if ($(this).html() == 'Tree') {
	  	scrollToElement(document.getElementById("tree_box"), undefined, undefined, undefined, undefined);//() => alert('done'))
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
							
							$scope.graphModel = buildGraph(data_col);
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
	$http.get('/stats_jlogs_times').success(function(data) {
		$scope.dates = data;
		$scope.model.selectedDate = data[0];
		$scope.dateListModel = [{'id':data[0]}];
		for (i = 0; i < $scope.dates.length; i++) {
			$scope.dateListData.push({'id':i+1, 'label':data[i]});
			$scope.compare2DateList.push({'id':i+1, 'label':data[i]});
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
//			console.log("\nlink "+lnk.id+":"+JSON.stringify(lnk.source.name)+">"+JSON.stringify(lnk.target.name)+", "+lnk.visible+" has protos/funcs "+JSON.stringify(link_protos)+", "+JSON.stringify(link_funcs)+" comes as visible: "+lnk.visible);

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
//					console.log(JSON.stringify(scope.filterListModel));
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
	return $.map(dict, function(value, key) { return value });
}
