# Queue and resume

## The queue

When you press **Post Files** on several tabs, the posts wait for their turn and are uploaded one after the other. The **pause** button stops the uploads and starts them again.

## Saved queue

By default the posts that are not finished are saved. If you close ngPost, or the computer stops, at the next start a window shows how many posts were not finished:

- **Resume**: they are started again. A post that was being sent starts again **from the beginning**.
- **Discard**: they are removed.
- **Decide later**: they stay saved and will be offered again at the next start.

The resumed posts appear as Quick Post tabs.

## Good to know

- The pause is not remembered after a restart.
- The saved queue also contains the password of each archive, in clear text, like the History. You can turn it off in **Settings**, section **Queue of the posts**: this also deletes the saved queue.
- On the command line use `--resume_queue`.
