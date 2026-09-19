# Replies

The mod that allows you to reply to comments on levels in Geometry Dash, nothing much to say.


## Features

- Replying to comments.
- Authentication system that doesn't rely on entering your password anywhere, just sending a gd message (automatically, of course)
- The replies are laid out in branches, so you can easily view multiple layers of replies at once. Sort of like Reddit.
- Real dates. Replies posted through the mod carry an exact timestamp, and the
  root comment gets one too — Geometry Dash's server sends an exact upload time
  with every comment, the game just throws it away. Switch `Date Display` to
  `Exact` or `Exact + relative` in the mod's settings.
- Reply IDs on cells, via `Show Reply IDs`.
- A `Reply Info` popup on every cell: tap the date line for the full ID, the
  local and UTC timestamps, the author's account ID, likes, the level ID and a
  copy button.

## Credits

The trick used to recover exact comment dates — hooking `GJComment::create` to
keep the timestamp before the game discards it — is the same one
[BetterInfo](https://github.com/Cvolton/betterinfo-geode) by Cvolton uses for
its own "Show Exact Comment Dates" feature.
