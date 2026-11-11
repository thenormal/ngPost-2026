# Failed articles and repost

During an upload some articles can fail (a connection drops, the server refuses them). ngPost tries each article several times. If some still fail, the post ends in orange and the log says how many articles failed.

## Repost missing articles

When some articles failed, the tab shows a button **Repost missing articles**. It posts **only those articles** again and fixes the NZB file. You do not need to send the whole post again.

After a successful repost the list of files is updated and the post turns green.

## Tips

- If many articles fail, the cause is usually the connection. Lower the number of connections, or check the provider limits.
- If the same articles fail again and again, check your account and the newsgroups.
- The number of retries can be set with the option `--retry` (default 5) or in the configuration file.

See [Servers and connections](02-servers.md).
