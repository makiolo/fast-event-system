#include <boost/bind.hpp>
#include <boost/coroutine/coroutine.hpp>
#include <boost/filesystem.hpp>
#include <coro/pipeline.h>
#include <coro/cmd.h>
#include <asyncply/parallel.h>
#include <libssh/libssh.h>
#include "../async_fast.h"

int show_remote_processes(ssh_session session)
{
	ssh_channel channel;
	int rc;
	char buffer[BUFSIZ];
	int nbytes;
	channel = ssh_channel_new(session);
	if (channel == NULL)
		return SSH_ERROR;
	rc = ssh_channel_open_session(channel);
	if (rc != SSH_OK)
	{
		ssh_channel_free(channel);
		return rc;
	}
	rc = ssh_channel_request_exec(channel, "ls -l");
	if (rc != SSH_OK)
	{
		ssh_channel_close(channel);
		ssh_channel_free(channel);
		return rc;
	}
	nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
	while (nbytes > 0)
	{
		if (write(1, buffer, nbytes) != nbytes)
		{
			ssh_channel_close(channel);
			ssh_channel_free(channel);
			return SSH_ERROR;
		}
		nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
		std::cout << buffer << std::endl;
	}

	if (nbytes < 0)
	{
		ssh_channel_close(channel);
		ssh_channel_free(channel);
		return SSH_ERROR;
	}
	ssh_channel_send_eof(channel);
	ssh_channel_close(channel);
	ssh_channel_free(channel);
	return SSH_OK;
}

int verify_knownhost(ssh_session session)
{
	int state, hlen;
	unsigned char* hash = NULL;
	char* hexa;
	char buf[10];
	state = ssh_is_server_known(session);
	hlen = ssh_get_pubkey_hash(session, &hash);
	if (hlen < 0)
		return -1;
	switch (state)
	{
		case SSH_SERVER_KNOWN_OK:
			break; /* ok */
		case SSH_SERVER_KNOWN_CHANGED:
			fprintf(stderr, "Host key for server changed: it is now:\n");
			ssh_print_hexa("Public key hash", hash, hlen);
			fprintf(stderr, "For security reasons, connection will be stopped\n");
			free(hash);
			return -1;
		case SSH_SERVER_FOUND_OTHER:
			fprintf(stderr, "The host key for this server was not found but an other"
							"type of key exists.\n");
			fprintf(stderr, "An attacker might change the default server key to"
							"confuse your client into thinking the key does not exist\n");
			free(hash);
			return -1;
		case SSH_SERVER_FILE_NOT_FOUND:
			fprintf(stderr, "Could not find known host file.\n");
			fprintf(stderr, "If you accept the host key here, the file will be"
							"automatically created.\n");
		/* fallback to SSH_SERVER_NOT_KNOWN behavior */
		case SSH_SERVER_NOT_KNOWN:
			hexa = ssh_get_hexa(hash, hlen);
			fprintf(stderr, "The server is unknown. Do you trust the host key?\n");
			fprintf(stderr, "Public key hash: %s\n", hexa);
			free(hexa);
			if (fgets(buf, sizeof(buf), stdin) == NULL)
			{
				free(hash);
				return -1;
			}
			if (strncasecmp(buf, "yes", 3) != 0)
			{
				free(hash);
				return -1;
			}
			if (ssh_write_knownhost(session) < 0)
			{
				fprintf(stderr, "Error %s\n", strerror(errno));
				free(hash);
				return -1;
			}
			break;
		case SSH_SERVER_ERROR:
			fprintf(stderr, "Error %s", ssh_get_error(session));
			free(hash);
			return -1;
		default:
			fprintf(stderr, "Invalid case %s", ssh_get_error(session));
			free(hash);
			return -1;
	}
	free(hash);
	return 0;
}


int main()
{
	using namespace asyncply;
	std::cout.sync_with_stdio(false);

	std::vector<std::string> lines;
	pipe_string(find("../tests"), grep("test_"), out(lines));
	for (auto& line : lines)
		std::cout << line << std::endl;


	ssh_session my_ssh_session;
	int rc;
	char* password;
	// Open session and set options
	my_ssh_session = ssh_new();
	if (my_ssh_session == NULL)
		exit(-1);
	ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, "localhost");
	// Connect to server
	rc = ssh_connect(my_ssh_session);
	if (rc != SSH_OK)
	{
		fprintf(stderr, "Error connecting to localhost: %s\n", ssh_get_error(my_ssh_session));
		ssh_free(my_ssh_session);
		exit(-1);
	}
	// Verify the server's identity
	// For the source code of verify_knowhost(), check previous example
	if (verify_knownhost(my_ssh_session) < 0)
	{
		ssh_disconnect(my_ssh_session);
		ssh_free(my_ssh_session);
		exit(-1);
	}
	// Authenticate ourselves
	password = getpass("Password: ");
	rc = ssh_userauth_password(my_ssh_session, NULL, password);
	if (rc != SSH_AUTH_SUCCESS)
	{
		fprintf(stderr, "Error authenticating with password: %s\n", ssh_get_error(my_ssh_session));
		ssh_disconnect(my_ssh_session);
		ssh_free(my_ssh_session);
		exit(-1);
	}

	std::cout << "connected!" << std::endl;
	show_remote_processes(my_ssh_session);

	ssh_disconnect(my_ssh_session);
	ssh_free(my_ssh_session);


	//
	// fes::async_fast< std::shared_ptr<asyncply::coro<int> > > _channel;
	// std::atomic<bool> _exit;
	// _exit = false;
	// asyncply::parallel(
	// 		[&](){
	// 			while(!_exit)
	// 			{
	// 				_channel.connect([&](const std::shared_ptr<asyncply::coro<int> >& coro) {
	// 					while(*coro)
	// 					{
	// 						int x = coro->get();
	// 						_exit = (x == 1);
	// 						(*coro)();
	// 					}
	// 				});
	// 				_channel.update();
	// 			}
	// 		},
	// 		[&](){
	// 			asyncply::run(
	// 				[&](){
	// 					for(int i=0; i<1000; ++i)
	// 					{
	// 						_channel(asyncply::corun<int>(
	// 							[](asyncply::yield_type<int>& yield)
	// 							{
	// 								std::cout << "create " << std::endl;
	// 								yield(0);
	// 								std::cout << "download " << std::endl;
	// 								yield(0);
	// 								std::cout << "patching " << std::endl;
	// 								yield(0);
	// 								std::cout << "compile " << std::endl;
	// 								yield(0);
	// 								std::cout << "tests " << std::endl;
	// 								yield(0);
	// 								std::cout << "packing " << std::endl;
	// 								yield(0);
	// 							}
	// 						));
	// 					}
	// 				}
	// 				,
	// 				[&](){
	// 					_channel(asyncply::corun<int>(
	// 						[](asyncply::yield_type<int>& yield)
	// 						{
	// 							std::cout << "request exit" << std::endl;
	// 							yield(1);
	// 						}
	// 					));
	// 				}
	// 			);
	// 		},
	// 		[&](){
	// 			_channel(asyncply::corun<int>(
	// 				[](asyncply::yield_type<int>& yield)
	// 				{
	// 					std::cout << "step1 - thread3 " << std::endl;
	// 					yield(0);
	// 					std::cout << "step2 - thread3 " << std::endl;
	// 					yield(0);
	// 					std::cout << "step3 - thread3 " << std::endl;
	// 					yield(0);
	// 				}
	// 			));
	// 		}
	// );

	// pipelines in parallel
	// try
	// {
	// 	auto result = asyncply::parallel(
	// 		[](){
	// 			return "one";
	// 		},
	// 		[](){
	// 			cmd({
	// 				find(".."),
	// 				grep(".*\\.cpp$|.*\\.h$"),
	// 				cat(),
	// 				grep("class|struct|typedef|using|void|int|double|float"),
	// 				grep_v("enable_if|;|\"|\'"),
	// 				trim(),
	// 				split(" "),
	// 				uniq(),
	// 				join(" "),
	// 				out()
	// 			});
	// 			return "two";
	// 		}
	// 	);
	// 	std::cout << result.size() << std::endl;
	// 	for(auto& r : result)
	// 	{
	// 		std::cout << r << std::endl;
	// 	}
	// }
	// catch(boost::filesystem::filesystem_error& e)
	// {
	// 	std::cout << "exception: " << e.what() << std::endl;
	// }
}
