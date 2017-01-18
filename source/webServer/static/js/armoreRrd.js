
var graph_opts={
	legend: {
		noColumns: 4,
		backgroundColor: "#FFFFFF"
	},
	grid: {
		color: "#000000",
		backgroundColor: "#FFFFFF"
	}
};
/*
var ds_graph_opts = {
	'Oscilator': {
		color: "#ff8000", 
		lines: {
			show: true,
			fill: true,
			fillColor:"#ffff80"
		}
	},
	'Idle': {
		label: 'IdleJobs',
		color: "#00c0c0", 
		lines: {
			show: true,
			fill: true
		}
	},
	'Running': {
		color: "#000000",
		yaxis:2
	}
};
*/
var ds_graph_opts = {}

var rrdflot_defaults = {
	graph_width: Math.round(window.innerWidth * .7).toString() + "px",
	graph_height: Math.round(window.innerHeight * .66).toString() + "px"
};

flot_obj = new rrdFlotAsync("mygraph",null,null,graph_opts,ds_graph_opts, rrdflot_defaults);

// this function is invoked when the RRD file name changes
function fname_update() {
	var fname=document.getElementById("fname").value;
	rrdflot_defaults = {
		graph_width: Math.round(window.innerWidth * .7).toString() + "px",
		graph_height: Math.round(window.innerHeight * .66).toString() + "px"
	};
	flot_obj = new rrdFlotAsync("mygraph",null,null,graph_opts,ds_graph_opts, rrdflot_defaults);
	flot_obj.reload(fname);
	document.getElementById("fname").firstChild.data=fname;
}

function rrdOptions() {
	var selSourceOption = document.getElementById("selSource");
	var selSourceValue = selSourceOption[selSourceOption.selectedIndex].value;

	var selDestOption = document.getElementById("selDest_" + selSourceValue);
	var selDestValue = selDestOption[selDestOption.selectedIndex].value;
	var selDestArr = document.getElementsByName("rrdDest");
	for (i = 0; i < selDestArr.length; i++) {
		selDestArr[i].style.display = "none";
	}
	selDestOption.style.display = "inline";

	var selFuncOption = document.getElementById("selFunc_" + selSourceValue + "_" + selDestValue);
	var selFuncValue = selFuncOption[selFuncOption.selectedIndex].value;
	var selFuncArr = document.getElementsByName("rrdFunc");
	for (i = 0; i < selFuncArr.length; i++) {
		selFuncArr[i].style.display = "none";
	}
	selFuncOption.style.display = "inline";

	document.getElementById("fname").value = "/static/rrd/armore-" + selFuncOption.value;
}

function rrdSelect_Example() {

}
