function ReturnValue() {
    var num = helper.GetNumberOfParameters();
    var file_name = helper.GetParameter("file_name");
    var servers = helper.GetActiveAppServers();
    var server = 0;
    var dummy = "";
    for(server in servers) {
        dummy += servers[server]['name'];
        dummy += servers[server]['port'];
    }
    return(file_name+dummy);
//    var this_name = "";
//    for(var server in servers) [
//        this_name += servers[server]['name'];
 //       this_name = server;
  //  }
   // return(file_name+"____"+this_name);
}

ReturnValue();
