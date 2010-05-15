function ReturnValue() {
    var num = helper.GetNumberOfParameters();
    var file_name = helper.GetParameter("file_name");
    var servers = helper.GetActiveAppServers();
    var server;
    for(server in servers) {
        file_struct = helper.TryGetFileByName(servers[server], file_name);
        if (file_struct[0] == true) { // this is our file
            var IP = helper.GetIPAddress();
            var return_array = [true, IP, file_struct[2].toString(), file_struct[3].toString()];
            return(return_array);
        }
    }
// file not local to this machine
    var return_array = [false, "", "", ""];
    return(return_array);
}

ReturnValue();
