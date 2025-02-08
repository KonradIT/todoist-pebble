## Todoist Mini (Konrad's fork)

Github repository for the Pebble Watch app Todoist Mini

Todoist Mini is an unofficial pebble watch app for the popular productivity software "Todoist".

## Features

* Log in to your account via google or standard Todoist login.
* Scrolling text for items that overflow the width of the screen, the scroll speed is customizable via the config screen.
* Completely customizable colors! Create your own style of interface using any of Pebble's 64 supported colors.
* View and complete items from any of your Todoist projects.
* Supports indentation, due dates, and completion of recurring tasks.
* Pins all items with due dates from your Todoist account to the timeline.
* Add new items to your projects using the microphone on devices that support it.

## Development

I recommend using the Devcontainer repo: https://github.com/FBarrca/pebble-devcontainer

Thanks to Eric Migicovsky for mentioning it in his blog: https://ericmigi.com/blog/how-to-help-build-open-source-pebble-software

It makes things so much easier when developing on Windows, no need to mess with WSL yourself at all.

```bash

# in the devcontainer:
pebble clean
pebble build

# on the host machine:
adb push build/todoist-pebble.pbw /sdcard/Download/
```

I use the excellent "Sideload Helper" app to install the app on my watch. Get it: https://play.google.com/store/apps/details?id=io.rebble.charon&hl=en

## Contact
* email:bradpaugh@gmail.com