var width = 960,
	height = 800,
	radius = 10;

var color = d3.scale.category20();

var div = d3.select("#divVisContent")
	.append("div")  
	.attr({
		"class": "tooltip",
		"id": "divVis"})
	.style("opacity", 0); 

var force = d3.layout.force()
	.linkDistance(200)
	.size([width, height]);

var svg = d3.select("#divVisContent").append("svg")
	.attr("width", width)
	.attr("height", height);

var path = "data.json";
d3.json(path, function(error, graph) {
	if (error) throw error;

	var k = Math.sqrt(graph.nodes.length / (width * height));

	force
		.charge(-10 / k)
		.gravity(100 * k);

	force
		.nodes(graph.nodes)
		.links(graph.links)
		.start()

	var link = svg.selectAll(".link")
		.data(graph.links)
		.enter().append("line")
		.attr("class", "link")
		.style("stroke-width", function(d) { return Math.atan2(d.value/100, 1)*2; });

	var node = svg.selectAll(".node")
		.data(graph.nodes)
		.enter().append("circle")
		.attr("class", "node")
		.attr("r", function(d) { return Math.atan2(d.size, 10)*5; })
		.style("fill", function(d) { return color(d.group); })
		.call(force.drag);

	node.on("mouseover", function(d) {
		div.transition()
			.duration(200)
			.style("opacity", 0);
		div.transition()
			.duration(200)
			.style("opacity", 0.8);
		div.html(
			"IP: " + d.name + "<br/>"
			+ "pkt count: " + d.size + "<br/>")
			.style("left", (d3.event.pageX+10) + "px")
			.style("top", (d3.event.pageY-10) + "px");
	});

	link.on("mouseover", function(d) {
		div.transition()
			.duration(200)
			.style("opacity", 0);

		div.transition()
			.duration(200)
			.style("opacity", 0.8);

		var protos = "";
		$.each(d.PROTOCOL, function(i, item) {
			protos = protos + item.name + "<br/>"
		});
		div.html(
			"protocols: " + protos)
			.style("left", (d3.event.pageX+10) + "px")
			.style("top", (d3.event.pageY-10) + "px");
	});

	force.on("tick", function() {
		node.attr("cx", function(d) { return d.x = Math.max(radius, Math.min(width - radius, d.x)); })
			.attr("cy", function(d) { return d.y = Math.max(radius, Math.min(height - radius, d.y)); });
		link.attr("x1", function(d) { return d.source.x; })
			.attr("y1", function(d) { return d.source.y; })
			.attr("x2", function(d) { return d.target.x; })
			.attr("y2", function(d) { return d.target.y; });
	});
	
	/*force.on("tick", updateAttrs);
	force.on("end", updateAttrs);

	force
		.charge(-10 / k)
		.gravity(100 * k);

	force
		.nodes(graph.nodes)
		.links(graph.links);
	
	force.linkDistance(width/2);

	force.start();*/
});

