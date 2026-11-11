# Servers and connections

A server is your Usenet provider. You can enter several: the posts are uploaded on all the enabled servers.

## Fields

- **Host** and **Port**: given by your provider. The usual ports are 563 (or 443) with SSL and 119 without.
- **SSL**: use it whenever your provider offers it. It encrypts the connection.
- **User name** and **Password**: your account.
- **Connections**: how many connections are opened in parallel. More connections can be faster, but your provider has a limit (often 20 to 50). Going over the limit gives errors.

## Test connection

Press **Test connection** before posting. The test connects and logs in, and tells you if the host, the port, the SSL setting or the credentials are wrong.

## Common errors

- *Cannot connect*: wrong host or port, no internet, or a firewall or VPN blocks the connection.
- *Authentication failed*: wrong user name or password. Retry only a few times: providers can temporarily block an account after many failed logins.
- *Too many connections*: lower the number of connections.

See also [Common problems](12-problems.md).
