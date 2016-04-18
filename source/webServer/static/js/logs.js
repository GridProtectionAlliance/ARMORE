var logCount = 0;

function LoadLogs()
{
	var url = "logs";
	
	$.getJSON(url, function(json)
	{
		logCount = 0;
		
		$("#logs_loading").show();
		$("#logs_loaded").hide();
		
		LoadLog(json, "armore", "#proxy_logs");
		LoadLog(json, "bro", "#bro_logs");
		LoadLog(json, "netmap", "#netmap_logs");
		LoadLog(json, "zmq", "#zmq_logs");
		
		$("#log_count").text(logCount);
		$("#logs_loaded").show();
		$("#logs_loading").hide();
	});
}

function LoadLog(json, type, target)
{			
	$(target + ' tr').remove(); 
	
	var log = json[type];
	var tr=[];
		
	for (var i = 0; i < log.length; i++)
	 {
		tr.push("<tr>");
		tr.push("<td>" + log[i] + "</td>" );
		tr.push("</tr>");
	}
	
	logCount += log.length;
	$(target).append($(tr.join('')));
}

$(document).ready(function()
{
	if (location.pathname.match('/.*logs.*/g')) {
		LoadLogs();
	}
});

