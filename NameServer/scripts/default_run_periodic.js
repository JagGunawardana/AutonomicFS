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

function GetReplicaCount(All_Servers, file_name) {
    var rep_count = 0;
    for(server in All_Servers) {
        if (server=='127.0.0.1') {
            continue;
        }
        var current_server=All_Servers[server];
        var remote_file;
        for(remote_file in current_server) {
            var remote_file_name=current_server[remote_file][1];
            if (file_name==remote_file_name) {
                rep_count += 1;
            }
        }
    }
    return(rep_count);
}

function GetReplicatingServerList(All_Servers, file_name) {
    replica_servers = Array();
    for(server in All_Servers) {
        if (server=='127.0.0.1') {
            continue;
        }
        var current_server=All_Servers[server];
        var remote_file;
        var has_file = false;
        for(remote_file in current_server) {
            var remote_file_name=current_server[remote_file][1];
            if (file_name==remote_file_name) {
                has_file = true;
                break;
            }
        }
        if (has_file) {
            replica_servers.push(server);
        }
    }
    return(replica_servers);
}

function GetFreeServerList(All_Servers, file_name) {
    free_servers = Array();
    for(server in All_Servers) {
        if (server=='127.0.0.1') {
            continue;
        }
        var current_server=All_Servers[server];
        var remote_file;
        var has_file = false;
        for(remote_file in current_server) {
            var remote_file_name=current_server[remote_file][1];
            if (file_name==remote_file_name) {
                has_file = true;
                break;
            }
        }
        if (!has_file) {
            free_servers.push(server);
        }
    }
    return(free_servers);
}

function ReplicateFile(file_name, server_to_rep) {
    var servers = helper.GetActiveAppServers();
    var server;
    var have_file = false;
    var file_content;
    for(server in servers) {
        var file_struct = helper.TryGetFileByName(servers[server], file_name);
        if (file_struct[0] == true) { // this is our file
            file_content = file_struct[3];
            have_file = true;
            break;
        }
    }
    if (!have_file) {
        helper.LogMessage("Failed to replicate file: "+file_name);
    }
    else {
        helper.SaveRemoteFile(server_to_rep, file_name, file_content, true);
        helper.LogMessage("Replicated file "+file_name);
    }
}

function DeleteLocalFile(file_name) {
    var servers = helper.GetActiveAppServers();
    var server;
    for(server in servers) {
       helper.DeleteFile(servers[server], file_name);
       helper.LogMessage("Removing local file "+file_name);
    }
}

function Replication(All_Servers) {
    var our_server=All_Servers['127.0.0.1'];
    var replica_count = helper.GetReplicaCount();
    for(file in our_server) {
        var file_name=our_server[file][1];
        var rep_count = GetReplicaCount(All_Servers, file_name);
        helper.LogMessage("File: "+file_name+", has replica count: "+rep_count);
        if (replica_count>rep_count) {
            // Need to replicate once (by pushing it to the best choice server)
            var free_servers = GetFreeServerList(All_Servers, file_name);
            if (free_servers.length==0) {
                helper.LogMessage("No free servers to replicate file: "+file_name);
            }
            else {
                var server_to_rep = helper.GetIdleNameServerFromList(free_servers);
                if (server_to_rep.length>0) {
                    helper.LogMessage("Replicating "+file_name+" to server "+server_to_rep);
                    ReplicateFile(file_name, server_to_rep);
                }
                else {
                    helper.LogMessage("No free idle servers to replicate file: "+file_name);
                }
            }
        }
        else if (replica_count<rep_count) {
            // Need to drop a replica (drop ours if we're least busy)
            var replicating_servers = GetReplicatingServerList(All_Servers, file_name);
            var server_to_drop = helper.GetIdleNameServerFromList(replicating_servers);
            if (server_to_drop.length>0) {
                var our_load = helper.GetLoadOfServer("127.0.0.1");
                var drop_load = helper.GetLoadOfServer(server_to_drop);
                if (our_load<drop_load) {
                    helper.LogMessage("Removing local file: "+file_name);
                    DeleteLocalFile(file_name);
                }
                else {
                    helper.LogMessage("Not able to pick a server (loads same) to remove for file: "+file_name);
                }
            }
            else {
                helper.LogMessage("Not able to pick a server to remove for file: "+file_name);
            }
        }
    }
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
                //helper.Debug("Looking for file name:"+file_name+", hash:"+file_hash+", tstamp:"+file_tstamp);
                //helper.Debug("Comparing with file name:"+remote_file_name+", hash:"+remote_file_hash+", tstamp:"+remote_file_tstamp+", remote IP: "+remote_ip);
                if (file_name==remote_file_name && file_tstamp<=remote_file_tstamp && file_hash!=remote_file_hash) {
                    // Get the remote file and save it on this server
                    var file_struct = helper.GetRemoteFileFromServerByName(remote_ip, remote_file_name);
                    if (file_struct[0]==true) {
                        var file_content = file_struct[3];
                        SaveLocalFile(file_name, file_content);
                        helper.LogMessage("Updated file "+file_name);
                    }
                    num_updates+=1;
                }
            }
        }
    }
    if (num_updates>0) {
        helper.LogMessage("Completed update cycle. Number of updates: "+num_updates);
    }
}

function SaveLocalFile(file_name, file_content) {
    var servers = helper.GetActiveAppServers();
    var server;
    var ret_val = false;
    for(server in servers) {
        if (helper.SaveFile(servers[server], file_name, file_content) == true) {
            ret_val = true;
        }
    }
    return(ret_val);
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
    Replication(All_Servers);
