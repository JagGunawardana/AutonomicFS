function ReturnValue() {
    var num = helper.GetNumberOfParameters();
    var file_name = helper.GetParameter("file_name");
    helper.Debug("Trying to get file "+file_name);
    var servers = helper.GetActiveAppServers();
    var server;
    var file_struct;
    var return_array;
    for(server in servers) {
        helper.Debug("Trying to get file from "+servers[server]["name"]);
        file_struct = helper.TryGetFileByName(servers[server], file_name);
        if (file_struct[0] == true) { // this is our file
            var IP = helper.GetIPAddress();
            return_array = [true, IP, file_struct[3].toString(), file_struct[2].toString()];
            return(return_array);
        }
    }
    helper.Debug("Can't file file locally");
    return(CheckRemoteNS(file_name));
}

function CheckRemoteNS(var file_name) {
    var servers = helper.GetActiveNameServers();
    var server;
    var ret_val = new Array();
    var file_struct;
    var return_array;
    for(server in servers) {
        file_struct = helper.GetRemoteFileFromServer(servers[server], file_name);
        if (file_struct[0] == true) { // this is our file
            var IP = servers[server];
            return_array = [true, IP, file_struct[3].toString(), file_struct[2].toString()];
            return(return_array);
        }
    }
// file not anywhere ???
    return_array = [false, "", "", ""];
    return(return_array);
}

helper.Debug("Trying to get file");
ReturnValue();
