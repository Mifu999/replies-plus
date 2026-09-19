# Replies
A Geode mod that adds threaded replies to Geometry Dash comments.

<img src="logo.png" width="150" alt="the mod's logo" />

## What each source file does

| File | Purpose |
| --- | --- |
| `src/main.cpp` | Hooks `CommentCell::loadFromComment` to put the reply button on vanilla comments |
| `src/ReplyLayer.cpp` | The popup: paging, the reply box, the scroll layer, the branch tree |
| `src/ReplyCell.cpp` | One row — icon, author, content, likes, the branch sprites, the info row |
| `src/ReplyInfoPopup.hpp` | The "Reply Info" popup and the clipboard dump |
| `src/CommentTimestamp.*` | Hooks `GJComment::create` to keep the exact upload timestamp |
| `src/TimeUtils.hpp` | `strftime`-based date formatting, with a sanity guard on timestamps |
| `src/Structs.hpp` | The `Reply` model, its JSON serialisation, and the display helpers |
| `src/Auth.cpp` | The GD-message-based login flow |

## Notes for contributors

- Requires **C++23**, as Geode v5 does.
- Every node in a reply cell carries a `cdc.replies/`-prefixed node ID, so other
  mods can find them without walking the child list by index.
- Exact comment dates come from key `15` of the comment object, which the
  official [gd-docs](https://github.com/gd-docs/gd-docs) comment table does not
  list yet — it is a newer addition on RobTop's side. The code therefore treats
  a missing or implausible value as "no date" and falls back to the relative
  string rather than rendering something wrong.

## Getting started
We recommend heading over to [the getting started section on our docs](https://docs.geode-sdk.org/getting-started/) for useful info on what to do next.

## Build instructions
For more info, see [our docs](https://docs.geode-sdk.org/getting-started/create-mod#build)
```sh
# Assuming you have the Geode CLI set up already
geode build
```

# Resources
* [Geode SDK Documentation](https://docs.geode-sdk.org/)
* [Geode SDK Source Code](https://github.com/geode-sdk/geode/)
* [Geode CLI](https://github.com/geode-sdk/cli)
* [Bindings](https://github.com/geode-sdk/bindings/)
* [Dev Tools](https://github.com/geode-sdk/DevTools)
