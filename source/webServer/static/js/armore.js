
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
    var $content = $("#psdash");
    $content.on("change", "#connections-form select", function () {
        $content.find("#connections-form").submit();
    });
    $content.on("focus", "#connections-form select, #connections-form input", function () {
        skip_updates = true;
    });
    $content.on("blur", "#connections-form select, #connections-form input", function () {
        skip_updates = false;
    });
    $content.on("keypress", "#connections-form input[type='text']", function (e) {
        if (e.which == 13) {
            $content.find("#connections-form").submit();
        }
    });
}

$(document).ready(function() {
    init_connections_filter();

    if($("#log").length == 0) {
        init_updater();
    } else {
        init_log();
    }

	oneTimePageLoad();
});

function addDigest() {
	theForm = $('#frmAddUser');
	if (theForm.length == 0) {
		theForm = $("#frmUpdPwd");
		email = $("#spUsername").text();
		password = $("#inpNewPwd").val();
	} else {
		email = $("#inpAddUserEmail").val();
		password = $("#inpAddUserPwd").val();
	}

	realm =  "ARMORE";
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
	console.log(input);
	console.log(output);
}

function oneTimePageLoad() {

	opModeChanged();
	netRoleChanged();
	checkRedirect();
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

