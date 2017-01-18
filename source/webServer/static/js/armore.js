
function escape_regexp(str) {
  return str.replace(/[\-\[\]\/\{\}\(\)\*\+\?\.\\\^\$\|]/g, "\\$&");
}

function replace_all(find, replace, str) {
  return str.replace(new RegExp(escape_regexp(find), 'g'), replace);
}

function init_log() {
    var $log = $("#log");
    function scroll_down($el) {
        $el.scrollTop($el[0].scrollHeight);
    }

    function read_log() {
        var $el = $("#log-content");
        var mode = $el.data("mode");
        if(mode != "tail") {
            return;
        }

        $.get($log.data("read-log-url"), function (resp) {
            // only scroll down if the scroll is already at the bottom.
            if(($el.scrollTop() + $el.innerHeight()) >= $el[0].scrollHeight) {
                scroll_down($el);
            }
			$el.html($('<div/>').text(resp).html());
        });
    }

    function exit_search_mode() {
        var $el = $("#log-content");
        $el.data("mode", "tail");
        var $controls = $("#log").find(".controls");
        $controls.find(".mode-text").text("Tail mode (Press s to search)");
        $controls.find(".status-text").hide();

        $.get($log.data("read-log-tail-url"), function (resp) {
            $el.text(resp);
            scroll_down($el);
            $("#search-input").val("").blur();
        });
    }

    $("#scroll-down-btn").click(function() {
        scroll_down($el);
    });

    $("#search-form").submit(function(e) {
        e.preventDefault();

        var val = $("#search-input").val();
        if(!val) return;

        var $el = $("#log-content");
        var filename = $el.data("filename");
		var nextVal = $el.data("next");
		var lastSrch = $el.data("lastSrch");
        var params = {
            "filename": filename,
            "text": val,
			"next": ++nextVal,
			"lastSrch": lastSrch
        };
		$el.data('next',nextVal);

        $el.data("mode", "search");
        $("#log").find(".controls .mode-text").text("Search mode (Press enter for next, escape to exit)");

        $.get($log.data("search-log-url"), params, function (resp) {
            var $log = $("#log");
            $log.find(".controls .status-text").hide();
            $el.find(".found-text").removeClass("found-text");

            var $status = $log.find(".controls .status-text");

            if(resp.position == -1) {
                $status.text("EOF Reached.");
            } else {
                // split up the content on found pos.
                var content_before = resp.content.slice(0, resp.buffer_pos);
                var content_after = resp.content.slice(resp.buffer_pos + params["text"].length);

                // escape html in log content
                resp.content = $('<div/>').text(resp.content).html();

                // highlight matches
                var matched_text = '<span class="matching-text">' + params['text'] + '</span>';
                var found_text = '<span class="found-text">' + params["text"] + '</span>';
                content_before = replace_all(params["text"], matched_text, content_before);
                content_after = replace_all(params["text"], matched_text, content_after);
                resp.content = content_before + found_text + content_after;
                $el.html(resp.content);

                $status.text("Position " + resp.position + " of " + resp.filesize + ".");
				$(".found-text")[0].scrollIntoView(true);
            }

            $status.show();
        },"json");
    });

    $(document).keyup(function(e) {
        var mode = $el.data("mode");
        if(mode != "search" && e.which == 83) {
            $("#search-input").focus();
        }
        // Exit search mode if escape is pressed.
        else if(mode == "search" && e.which == 27) {
            exit_search_mode();
        }
    });

	$("#search-input").on('input', function() {
		$el.data('next', -1);
	});


    setInterval(read_log, 1000);
    var $el = $("#log-content");
    scroll_down($el);
}

var skip_updates = true;

function init_updater() {
    function update() {
        if (skip_updates) return;

        $.ajax({
            url: location.href,
            cache: false,
            dataType: "html",
            success: function(resp){
                $("#psdash").find(".main-content").html(resp);
            }
        });
    }

    setInterval(update, 3000);
}

function init_connections_filter() {
    var $theContent = $("#psdash");
    $theContent.on("change", "#connections-form select", function () {
        $theContent.find("#connections-form").submit();
    });
    $theContent.on("focus", "#connections-form select, #connections-form input", function () {
        skip_updates = true;
    });
    $theContent.on("blur", "#connections-form select, #connections-form input", function () {
        skip_updates = false;
    });
    $theContent.on("keypress", "#connections-form input[type='text']", function (e) {
        if (e.which == 13) {
            $theContent.find("#connections-form").submit();
        }
    });
}

$(document).ready(function() {
    //init_connections_filter();

    if (document.location.pathname != "/viewLog") {
      if($("#log").length == 0 ) {
	  init_updater();
      } else {
	  init_log();
      }
    }

	oneTimePageLoad();
});

function addDigest() {
	theForm = $('#frmAddUser');
	realm =  "ARMORE";
	if (theForm.length == 0) {
		theForm = $("#frmUpdPwd");
		email = $("#spUsername").text();
		password = $("#inpNewPwd").val();
		oldPassword = $("#inpOldPwd").val();
		md5_Digest_Old = md5(email + ":" + realm + ":" + oldPassword);
		theForm.append('<input type="hidden" name="md5_Digest_Old" value="' + md5_Digest_Old + '" />');
	} else {
		email = $("#inpAddUserEmail").val();
		password = $("#inpAddUserPwd").val();
	}

	digest = email + ":" + realm + ":" + password;
	md5_Digest = md5(digest);

	theForm.append('<input type="hidden" name="md5_Digest" value="' + md5_Digest + '" />');

	return true;
}

function checkUsername() {
	username = $("#inpAddUserEmail").val();

	if ($("#divAddUserEmail").html() == "") {
		$.ajax({
			url: "/admin/checkUsername?username=" + username,
			cache: false,
			dataType: "text",
			success: function(resp){
				$("#divAddUserEmail").html(resp);
				if (resp == 'Username OK') {
					$("#divAddUserEmail").addClass('success');
					$("#divAddUserEmail").removeClass('error');
				} else {
					$("#divAddUserEmail").removeClass('success');
					$("#divAddUserEmail").addClass('error');
				}
			}
		});
	}
}

function checkMD5() {
	input = $("#inpMd5").val();
	output = md5(input);
}

function oneTimePageLoad() {

	netRoleChanged();
	checkRedirect();

	if (location.pathname == "/statistics") {
		rrdOptions();
		fname_update();
	}

	if (location.pathname == "/settings") {
	  opModeChanged();
	}

	if (location.pathname == "/policy/add" || /\/policy\/edit/.test(location.pathname)) {
		//updateTimePeriodCheckBoxes(document.getElementById("inpPerDaily"));
		addPolicyInit();
	}

}

function opModeChanged() {

	var radProxy = $("#radProxy")[0];
	var radPassive = $("#radPassive")[0];
	var radTransparent= $("#radTransparent")[0];

	var proxyStatus = false;
	var passiveStatus = false;
	var transparentStatus = false;

	if (radProxy != null) {
		proxyStatus = radProxy.checked;
	}
	if (radPassive != null) {
		passiveStatus = radPassive.checked;
	}
	if (radTransparent != null) {
		transparentStatus = radTransparent.checked;
	}

	var proxyBox = $("#boxProxy");
	var passBox = $("#boxPassive");
	var transpBox = $("#boxTransparent");
	var broBox = $("#boxBro");
	
	if (proxyStatus) {
		proxyBox.show();
		broBox.show();

		transpBox.hide();
		passBox.hide()
	} else if (passiveStatus) {
		passBox.show();
		broBox.show();

		transpBox.hide();
		proxyBox.hide();
	} else if (transparentStatus) {
		transpBox.show();
		broBox.show();

		proxyBox.hide();
		passBox.hide();
	} else {
		transpBox.hide();
		proxyBox.hide();
		passBox.hide();
	}

}

function netRoleChanged() {

	var radServer = $("#radServer")[0];
	var radClient = $("#radClient")[0];

	var serverStatus = false;
	var clientStatus = false;
	
	if (radServer != null) {
		serverStatus = radServer.checked;
	}
	if (radClient != null) {
		clientStatus = radClient.checked;
	}

	targetIp = $("#tdTargetIp");
	targetPort = $("#tdTargetPort");

	if (serverStatus) {
		targetIp.css('visibility','hidden');
		targetPort.show();
	} else if (clientStatus) {
		targetIp.css('visibility','visible');
		targetPort.show();
	}
}

function redFunc(result) {
	setTimeout( function() {
		window.location = "http://" + $("#spRedirectDest").text();
	}, 10000);
}

function checkRedirect() {
	if ($('#redirectTo').length) {
		$.post("/restartServer");
		var redirectTo = "http://" + $("#spRedirectDest").text();
		var seconds = 0;
		var id = setInterval(function() {
			$.ajax({
				url: redirectTo,
				success: redFunc(),
				error: function(result) {
					seconds++;
					if (seconds >= 30) {
						alert(seconds + " seconds have passed without a response.  Giving up");
						clearInterval(id);
					}
				},
				statusCode: {
					401: redFunc()
				}
			});
		}, 1000);
	}
}

function updateTimePeriodCheckBoxes(theCheckbox) {

	var weekdays = ["Mon", "Tue", "Wed", "Thu", "Fri"];
	var weekends = ["Sat", "Sun"];
	var daily = weekdays.concat(weekends);

	if (theCheckbox.name == "Daily") {
		for (i = 0; i < daily.length; i++) {
			document.getElementsByName(daily[i])[0].checked = theCheckbox.checked;
		}
	} else if (theCheckbox.name == "Weekday") {
		for (i = 0; i < weekdays.length; i++) {
			document.getElementsByName(weekdays[i])[0].checked = theCheckbox.checked;
		}
	} else if (theCheckbox.name == "Weekend") {
		for (i = 0; i < weekends.length; i++) {
			document.getElementsByName(weekends[i])[0].checked = theCheckbox.checked;
		}
	}

	var numWeekdayChecked = 0;
	var numWeekendChecked = 0;
	for (i = 0; i < daily.length; i++) {
		var el = document.getElementsByName(daily[i])[0];
		if (weekdays.indexOf(el.name) != -1 && el.checked) {
			numWeekdayChecked++;
		}
		if (weekends.indexOf(el.name) != -1 && el.checked) {
			numWeekendChecked++;
		}
	}

	if (numWeekdayChecked == weekdays.length) {
		document.getElementsByName("Weekday")[0].checked = true;
	} else {
		document.getElementsByName("Weekday")[0].checked = false;
	}

	if (numWeekendChecked == weekends.length) {
		document.getElementsByName("Weekend")[0].checked = true;
	} else {
		document.getElementsByName("Weekend")[0].checked = false;
	}

	if ((numWeekdayChecked + numWeekendChecked) == daily.length) {
		document.getElementsByName("Daily")[0].checked = true;
	} else {
		document.getElementsByName("Daily")[0].checked = false;
	}
}

function policyAddTimeRow() {
	var timeRows = document.getElementById("divTimeRows");
	var newNum = timeRows.childNodes.length - 6; // 6 elements already exist in child nodes

	var newRow = document.getElementById("divTimeRow").cloneNode(true);
	newRow.id = "divTimeRow" + newNum
	newRow.childNodes[1].childNodes[1].name = "timeStart" + newNum; // Rename timeStart button
	newRow.childNodes[3].childNodes[1].name = "timeStop" + newNum; // Rename timeStop button
	var newRowChildren = newRow.childNodes;

	newRow.style.display = "inherit";
	timeRows.appendChild(newRow);
}

function policyRemoveTimeRow(currRow) {
	var timeRows = document.getElementById("divTimeRows");
	var thisRow = currRow.parentElement.parentElement;

	timeRows.removeChild(thisRow);
}

function deletePolicy(polNum) {
	return confirm('Are you sure you want to delete policy "' + polNum + '"?');
}

function deleteKey(name, type) {
	return confirm('Are you sure you want to delete ' + type + ' key "' + name + '"?');
}

function deleteUser(username) {
        return confirm("Are you sure you want to delete the user '" + username + "'?");
}

function updateField(theField, cBoxVal) {

	if (theField == "inpSrcSub" || theField == "inpDestSub") {
		if (cBoxVal) {
			document.getElementById(theField).value = "0.0.0.0/0";
		} else {
			document.getElementById(theField).value = "";
		}
	}

	if (theField == "inpFunc") {
		if (cBoxVal) {
			document.getElementById(theField).value = "*";
			document.getElementById("inpFuncRegexCont").checked = true;
		} else {
			document.getElementById(theField).value = "";
		}
	}

	if (theField == "inpTime0") {
		if (cBoxVal) {
			document.getElementById("inpTimeStart0").value = "00:00:00";
			document.getElementById("inpTimeStop0").value = "23:59:59";

			var timeRows = document.getElementById("divTimeRows");
			var newNum = timeRows.childNodes.length - 6; // 6 elements already exist in child nodes

			for (i = 1; i < newNum; i++) {
				document.getElementById("divTimeRow" + i).childNodes[5].childNodes[1].click();
			}

		} else {
			document.getElementById("inpTimeStart0").value = "";
			document.getElementById("inpTimeStop0").value = "";
		}
	}
}

function submitAcks() {
	cbs = document.getElementsByName("cbPolicyAck");
	cbsTrue = [];
	for (x = 0; x < cbs.length; x++) {
		if (cbs[x].checked) {
			cbsTrue.push(cbs[x].dataset.uid)
		}
	}
	$.post("/alerts/ack", {"uids": cbsTrue.join()});
	if (document.location.pathname == "/security") {
		window.location.href = "/security?filter=unacked";
	} else {
		window.location.reload();
	}
}

function updateFunction(val) {
	var inpFunc = document.getElementById("inpFunc");
	var cbAnyFunc = document.getElementById("cbFunc");
	if (val == "exact") {
		cbAnyFunc.checked = false;
		if (inpFunc.value == "*") {
			inpFunc.val = "";
		}
	}
	if (val == "contains" && inpFunc.val == "*") {
		inpFunc.val = "";
	}
}

function addPolicyInit() {

	var ids = ["SrcSub", "DestSub", "Func", "Time"];

	for (x = 0; x < ids.length; x++) {
		var inp = document.getElementById("inp" + ids[x]);
		var cb = document.getElementById("cb" + ids[x]);

		switch(ids[x]) {
			case "SrcSub":
			case "DestSub":
				if (/\/0$/.test(inp.value)) {
					cb.checked = true;
				} else {
					cb.checked = false;
				}
				break;
			case "Func":
				var cbExact = document.getElementById("inpFuncRegexExact");
				if (inp.value == "*") {
					if (!cbExact.checked) {

						cb.checked = true;
						document.getElementById("inpFuncRegexCont").checked = true;
					}
				} else {
					cb.checked = false;
				}
			case "Time":
				inpStart = document.getElementById("inpTimeStart0");
				inpStop = document.getElementById("inpTimeStop0");
				cb = document.getElementById("cbTimeAny");

				if (/0{1,2}:0{1,2}:0{1,2}/.test(inpStart.value) && inpStop.value == "23:59:59") {
					cb.checked = true;
				} else {
					cb.checked = false;
				}
		}
	}
}

