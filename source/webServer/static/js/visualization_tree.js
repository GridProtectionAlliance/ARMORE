var t_svg, t_root;

var i = 0,
    duration = 750;

// Calculate total nodes, max label length
var totalNodes = 0;
var maxLabelLength = 0;


// size variables
var margin = {top: 20, right: 120, bottom: 20, left: 120};
var t_width = $(document).width();
var t_height = $(document).height();
    //t_width = 960 - margin.right - margin.left,
    //t_height = 800 - margin.top - margin.bottom;

var tree = d3.layout.tree()
    //.separation(function(a,b){return ((a.parent == root) && (b.parent == root)) ? 1 : 6;})
    //.separation(function(a,b){return a.parent === b.parent ? 1 : 5;})
    .size([t_height, t_width]);

var diagonal = d3.svg.diagonal()
    .projection(function(d) { return [d.y, d.x]; });


// A recursive helper function for performing some setup by walking through all nodes

function visit(parent, visitFn, childrenFn) {
    if (!parent) return;

    visitFn(parent);

    var children = childrenFn(parent);
    if (children) {
        var count = children.length;
        for (var i = 0; i < count; i++) {
            visit(children[i], visitFn, childrenFn);
        }
    }
}



function update_tree(source) {

  // Compute the new tree layout.
  var t_nodes = tree.nodes(t_root).reverse(),
      t_links = tree.links(t_nodes);

  // Normalize for fixed-depth.
  //t_nodes.forEach(function(d) { d.y = d.depth * 180; });

  // Set widths between levels based on maxLabelLength.
  t_nodes.forEach(function(d) {
      d.y = (d.depth * (maxLabelLength * 4)); //maxLabelLength * 10px
      // alternatively to keep a fixed scale one can set a fixed depth per level
      // Normalize for fixed-depth by commenting out below line
      // d.y = (d.depth * 500); //500px per level.
  });

  // Update the nodes…
  var t_node = t_svg.selectAll("g.tree_node")
      .data(t_nodes, function(d) { return d.id || (d.id = ++i); });

  // Enter any new nodes at the parent's previous position.
  var nodeEnter = t_node.enter().append("g")
      .attr("class", "tree_node")
      .attr("transform", function(d) { return "translate(" + source.y0 + "," + source.x0 + ")"; })
      .on("click", tree_click);

  nodeEnter.append("circle")
      .attr("r", 1e-6)
      .style("fill", function(d) { return d._children ? "lightsteelblue" : "#fff"; });

  nodeEnter.append("text")
      .attr("x", function(d) { return d.children || d._children ? -10 : 10; })
      .attr("dy", ".35em")
      .attr("text-anchor", function(d) { return d.children || d._children ? "end" : "start"; })
      .text(function(d) { return d.name; })
      .style("fill-opacity", 1e-6);

  // Transition nodes to their new position.
  var nodeUpdate = t_node.transition()
      .duration(duration)
      .attr("transform", function(d) { return "translate(" + d.y + "," + d.x + ")"; });

  nodeUpdate.select("circle")
      .attr("r", 4.5)
      .style("fill", function(d) { return d._children ? "lightsteelblue" : "#fff"; });

  nodeUpdate.select("text")
      .style("fill-opacity", 1);

  // Transition exiting nodes to the parent's new position.
  var nodeExit = t_node.exit().transition()
      .duration(duration)
      .attr("transform", function(d) { return "translate(" + source.y + "," + source.x + ")"; })
      .remove();

  nodeExit.select("circle")
      .attr("r", 1e-6);

  nodeExit.select("text")
      .style("fill-opacity", 1e-6);

  // Update the links…
  var t_link = t_svg.selectAll("path.tree_link")
      .data(t_links, function(d) { return d.target.id; });

  // Enter any new links at the parent's previous position.
  t_link.enter().insert("path", "g")
      .attr("class", "tree_link")
      .attr("d", function(d) {
        var o = {x: source.x0, y: source.y0};
        return diagonal({source: o, target: o});
      });

  // Transition links to their new position.
  t_link.transition()
      .duration(duration)
      .attr("d", diagonal);

  // Transition exiting nodes to the parent's new position.
  t_link.exit().transition()
      .duration(duration)
      .attr("d", function(d) {
        var o = {x: source.x, y: source.y};
        return diagonal({source: o, target: o});
      })
      .remove();

  // Stash the old positions for transition.
  t_nodes.forEach(function(d) {
    d.x0 = d.x;
    d.y0 = d.y;
  });
}

// Toggle children on click.
function tree_click(d) {
  if (d.children) {
    d._children = d.children;
    d.children = null;
  } else {
    d.children = d._children;
    d._children = null;
  }
  update_tree(d);
}

function buildTree(flare) {
	console.log("Building tree...");
  // Call visit function to establish maxLabelLength
  visit(flare, function(d) {
      totalNodes++;
      if (d.name) {
        maxLabelLength = Math.max(d.name.length, maxLabelLength);
      }
  }, function(d) {
      return d.children && d.children.length > 0 ? d.children : null;
  });

  t_root = flare;
  t_root.x0 = t_height / 2;
  t_root.y0 = 0;

  if (t_root.children) {
    t_root.children.forEach(collapse);
    update_tree(t_root);
  }
//	d3.select(self.frameElement).style("t_height", "800px");
}

function collapse(d) {
  if (d.children) {
    d._children = d.children;
    d._children.forEach(collapse);
    d.children = null;
  }
}

function expand(d){   
  var children = (d.children)?d.children:d._children;
  if (d._children) {        
      d.children = d._children;
      d._children = null;       
  }
  if(children)
    children.forEach(expand);
}

function expandAll(){
  expand(t_root); 
  update_tree(t_root);
}

function collapseAll(){
	if (t_root.children) {
		t_root.children.forEach(collapse);
	  collapse(t_root);
	  update_tree(t_root);	
	}
}
