function GetRemoteServer(server_ip) {
    var file_list = helper.GetLocalFileListForServer(server_ip);
    return(file_list);
}

function GetLocalServers() {
    var servers = helper.GetActiveAppServers();
    var server;
    var ret_val = new Array();
    for(server in servers) {
        var file_list = helper.GetAllLocalFilesUnderMgt(servers[server]);
        ret_val = ret_val.concat(file_list);
    }
    return(ret_val);
}
function UpdateOurFiles(All_Servers) {
    // Does a 'pull' update
    var file;
    var num_updates=0;
    var our_server=All_Servers['127.0.0.1'];
    for(file in our_server) {
        var file_name=our_server[file][1];
        var file_hash=our_server[file][0];
        var file_tstamp=parseInt(our_server[file][4]);
        // We need to update if file_name same, file_tstamp is greater and hash is not same
        var server;
        for(server in All_Servers) {
            if (server=='127.0.0.1') {
                continue;
            }
            var current_server=All_Servers[server];
            var remote_file;
            for(remote_file in current_server) {
                var remote_file_name=current_server[remote_file][1];
                var remote_file_hash=current_server[remote_file][0];
                var remote_file_tstamp=parseInt(current_server[remote_file][4]);
                var remote_ip=current_server[remote_file][3];
                helper.Debug("Looking for file name:"+file_name+", hash:"+file_hash+", tstamp:"+file_tstamp);
                helper.Debug("Comparing with file name:"+remote_file_name+", hash:"+remote_file_hash+", tstamp:"+remote_file_tstamp+", remote IP: "+remote_ip);
                if (file_name==remote_file_name && file_tstamp<=remote_file_tstamp && file_hash!=remote_file_hash) {
                    helper.Debug("Need to update file "+file_name+" from "+server);
                    //!!!
                    num_updates+=1;
                }
            }
        }
    }
    helper.Debug("Number of updates: "+num_updates);
}

// This script does the main admin procedures for the system.

var All_Servers = Array();

// 1) Get all the files that we have on our server.
    All_Servers['127.0.0.1'] = GetLocalServers();

// 2) Get the files that are on all the other servers
    var remote_servers = helper.GetActiveNameServers();
    var server;
    for(server in remote_servers) {
        All_Servers[remote_servers[server]] = GetRemoteServer(remote_servers[server]); 
    }

// 3) Does one of our files need to be updated? If so then update from one of the other servers.
    UpdateOurFiles(All_Servers);

// Do we need to replicate (or unreplicate a file)? If so then decide how to do this and then do it.

