function ReturnValue() {
    var num = helper.GetNumberOfParameters();
    var file_hash = helper.GetParameter("file_hash");
    var servers = helper.GetActiveAppServers();
    var server;
    for(server in servers) {
        file_struct = helper.TryGetFileByHash(servers[server], file_hash);
        if (file_struct[0] == true) { // this is our file
            var IP = helper.GetIPAddress();
            var return_array = [true, IP, file_struct[2].toString(), file_struct[3].toString()];
            return(return_array);
        }
    }
// Check the remote NSs
    return(CheckRemoteNS(file_hash));
}

function CheckRemoteNS(file_hash) {
    var servers = helper.GetActiveNameServers();
    var server;
    var ret_val = new Array();
    var file_struct;
    var return_array;
    for(server in servers) {
        file_struct = helper.GetRemoteFileFromServerByHash(servers[server], file_hash);
        if (file_struct[0] == true) { // this is our file
            var IP = servers[server];
            return_array = [true, IP, file_struct[2].toString(), file_struct[3].toString()];
            return(return_array);
        }
    }
// file not anywhere ???
    return_array = [false, "", "", ""];
    return(return_array);
}

ReturnValue();
