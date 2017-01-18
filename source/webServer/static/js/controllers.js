'use strict';

function GraphController($scope) {
	
	$scope.searchNode = function() {
		// Find the node
		var selectedVal = document.getElementById('search').value;
		var svg = d3.select("#svg")
		var node = svg.selectAll(".node");
		if (selectedVal == "none") {
			node.style("stroke", "white").style("stroke-width", "1");
		} else {
			var selected = node.filter(function(d, i) {
				return d["name"] != selectedVal;
			});
			selected.style("opacity", "0");
			var link = svg.selectAll(".link")
			link.style("opacity", "0");
			d3.selectAll(".node, .link").transition().duration(5000).style("opacity",
					1);
		}
	}
	
	$scope.get_protocols = function(links) {
		var protos = new Set();
		for (var i = 0; i < links.length; i++) {
			for ( var j in links[i].PROTOCOL) {
				console.log(links[i].PROTOCOL[j].name)
				protos.add(links[i].PROTOCOL[j].name);
			}
			;
		}
		;
		for (v in protos) {
			console.log(v);
		}
	};

};