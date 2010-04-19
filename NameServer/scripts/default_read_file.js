function ReturnValue() {
    var num = helper.GetNumberOfParameters();
    var file_name = helper.GetParameter("file_name");
    var servers = helper.GetActiveAppServers();
    var server = 0;
    var file = "";
    for(server in servers) {
        file = helper.TryGetFileByName(server, file_name);
    }
    if (file[0] == true) {
        return("Yes");
    }
    else {
        return("No");
    }
//    var this_name = "";
//    for(var server in servers) [
//        this_name += servers[server]['name'];
 //       this_name = server;
  //  }
   // return(file_name+"____"+this_name);
}

ReturnValue();
