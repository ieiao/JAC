const dt_sync_button = document.getElementById("dt_sync");
const config_exit_button = document.getElementById("config_exit");
const firstboot_button = document.getElementById("firstboot");
const upgrade_button = document.getElementById("upgrade");
const sleepset_button = document.getElementById("sleep_set");

function get_dt()
{
  dt = new Date();
  var dt_str = dt.getFullYear() + '/' + (dt.getMonth() + 1) + '/' + dt.getDate() + '-';
  if (dt.getDay() == 0) dt_str += '7';
  else dt_str += dt.getDay();
  dt_str += '-' + dt.getHours() + ':' + dt.getMinutes() + ':' + dt.getSeconds();
  return dt_str;
}

function sync_datetime()
{
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var message = "<div id='dt_sync_result' class='alert alert-success alert-dismissible fade show' style='margin:10px;'>";
      message += "<button type='button' class='close' data-dismiss='alert'>&times;</button>";
      message += "<strong>" + "同步成功" + "</strong></div>";
      document.getElementById('dt_sync_result_box').innerHTML = message;
      setTimeout(function() {$('#dt_sync_result').alert('close');}, 3000);
    } else {
      var message = "<div id='dt_sync_result' class='alert alert-danger alert-dismissible fade show' style='margin:10px;'>";
      message += "<button type='button' class='close' data-dismiss='alert'>&times;</button>";
      message += "<strong>" + "同步失败" + "</strong></div>";
      document.getElementById('dt_sync_result_box').innerHTML = message;
      setTimeout(function() {$('#dt_sync_result').alert('close');}, 3000);
    }
  };
  xhttp.open('GET', "set/dt?" + get_dt());
  xhttp.send();
}

function get_ps()
{
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var info = xhttp.responseText.split(" ");
      var begin = info[0];
      var end = info[1];
      var form = document.getElementById('form_sleep');
      form.begin.value = begin;
      form.end.value = end;
    }
  };
  xhttp.open('GET', "/get/ps");
  xhttp.send();
}

function get_version()
{
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("firmware_version").textContent = "固件版本: " + xhttp.responseText;
    }
  };
  xhttp.open('GET', "/get/version");
  xhttp.send();
}

dt_sync_button.addEventListener("click", async () => {
  try {
    sync_datetime();
  } catch(err) {
    console.error(err);
    alert("An error occured while fetching device details");
  }
});

config_exit_button.addEventListener("click", async () => {
  try {
    var xhttp = new XMLHttpRequest();
    xhttp.open('GET', "set/exit");
    xhttp.send();
  } catch(err) {
    console.error(err);
    alert("An error occured while fetching device details");
  }
});

firstboot_button.addEventListener("click", async () => {
  try {
    var xhttp = new XMLHttpRequest();
    xhttp.open('GET', "set/firstboot");
    xhttp.send();
  } catch(err) {
    console.error(err);
    alert("An error occured while fetching device details");
  }
});

upgrade_button.addEventListener("click", async () => {
  try {
    var otafile = document.getElementById("otafile").files;
    var file = otafile[0];

    if (file) {
      var xhttp = new XMLHttpRequest();

      document.getElementById("otafile").disabled = true;
      document.getElementById("upgrade").disabled = true;

      xhttp.onreadystatechange = function() {
        if (xhttp.readyState == 4) {
          if (xhttp.status == 200) {
            document.getElementById("progress").textContent = "传输成功, 设备重启";
            document.getElementById("otafile").disabled = false;
            document.getElementById("upgrade").disabled = false;
          } else {
            document.getElementById("progress").textContent = "传输异常";
            document.getElementById("otafile").disabled = false;
            document.getElementById("upgrade").disabled = false;
          }
        }
      };

      xhttp.upload.onprogress = function (e) {
        var progress = document.getElementById("progress");
        progress.textContent = "传输进度: " + (e.loaded / e.total * 100).toFixed(0) + "%";
      };

      xhttp.open('POST', "upgrade", true);
      xhttp.send(file);

    } else {
      document.getElementById("progress").textContent = "文件无效";
    }
  } catch(err) {
    console.error(err);
    alert("An error occured while fetching device details");
  }
});

sleepset_button.addEventListener("click", async () => {
  try {
    var form = document.getElementById('form_sleep');
    if (form.begin.value < 0 || form.begin.value > 23 || form.end.value < 0 || form.end.value > 23 || form.begin.value == form.end.value) {
      var message = "<div id='sleep_alert' class='alert alert-danger alert-dismissible fade show' style='margin:10px;'>";
      message += "<button type='button' class='close' data-dismiss='alert'>&times;</button>";
      message += "<strong>" + "无效的配置" + "</strong></div>";
      document.getElementById('sleep_set_result').innerHTML = message;
      setTimeout(function() {$('#sleep_alert').alert('close');}, 3000);
    } else {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var message = "<div id='sleep_alert' class='alert alert-success alert-dismissible fade show' style='margin:10px;'>";
          message += "<button type='button' class='close' data-dismiss='alert'>&times;</button>";
          message += "<strong>" + "保存成功" + "</strong></div>";
          document.getElementById('sleep_set_result').innerHTML = message;
          setTimeout(function() {$('#sleep_alert').alert('close');}, 3000);
        } else {
          var message = "<div id='sleep_alert' class='alert alert-danger alert-dismissible fade show' style='margin:10px;'>";
          message += "<button type='button' class='close' data-dismiss='alert'>&times;</button>";
          message += "<strong>" + "保存失败" + "</strong></div>";
          document.getElementById('sleep_set_result').innerHTML = message;
          setTimeout(function() {$('#sleep_alert').alert('close');}, 3000);
        }
      };
      xhttp.open('GET', "/set/ps?begin=" + form.begin.value + "&end=" + form.end.value);
      xhttp.send();
    }
  } catch(err) {
    console.error(err);
    alert("An error occured while fetching device details");
  }
});

setTimeout(function() {sync_datetime();}, 500);
setTimeout(function() {get_ps();}, 500);
setTimeout(function() {get_version();}, 500);
