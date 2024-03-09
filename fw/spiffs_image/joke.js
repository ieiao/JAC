const dt_sync_button = document.getElementById("dt_sync");
const config_exit_button = document.getElementById("config_exit");
const firstboot_button = document.getElementById("firstboot");

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

setTimeout(function() {sync_datetime();}, 500);
