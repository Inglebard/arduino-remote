function sendCommand(protocol,address,command,repeat)
{
  $.ajax({
    url: "/send/?protocol="+protocol+"&address="+address+"&command="+command+"&repeat="+repeat
  });  
}

$(document).ready(function() {
  $(".sendCommand").click(function(e) {
    e.preventDefault();
    var parameter = $(this).data("parameter");

    sendCommand(parameter.protocol,parameter.address,parameter.command,parameter.repeat);
  });
  
});