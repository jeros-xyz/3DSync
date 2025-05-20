# JerosSaveSync

Homebrew for Nintendo 3DS/2DS console family that allows synchronization of saves and files to a cloud, to another console or to a PC

Warning : Download only tranfert into /3ds folder of your Sd card

**currently in beta, supports only Dropbox**

## Compilation of 3ds file for linux

1. Install DevKitPro and submodule:
   1. Download and install kit : `wget https://apt.devkitpro.org/install-devkitpro-pacman && chmod +x ./install-devkitpro-pacman && sudo ./install-devkitpro-pacman`
   2. Install Submodule : `sudo dkp-pacman -S 3ds-curl`
   3. Install Submodule : `sudo dkp-pacman -S smdh`
   4. Install Submodule : `sudo dkp-pacman -S 3ds-smdh`
   5. Install Submodule : `sudo dkp-pacman -S 3ds-bannertool`
   6. Install makerom (found on the web, must be launch by command `makerom`)
2. Export devkitpro path : `export DEVKITPRO=/opt/devkitpro`
3. Create a Dropbox developper application
   1. Visit this url for create application : https://www.dropbox.com/developers/apps/create
   2. Into Settings, get `Appkey` and `Appsecret`
   3. Into Permissions, check `files.content.read` and `files.content.write`
4. Visit the url for Authorization code (Warning : valid once)
   1. Go to "https://www.dropbox.com/oauth2/authorize?client_id=`Appkey`&token_access_type=offline&response_type=code"
   2. Copy `Authorisation code` displayed on webpage
5. Take `sample/JerosSaveSync.ini` as sample, modify it and transfert it to your 3ds :
   1. Replace `AuthorizationCode` into file by `Authorisation code`
   2. Transfert result file into your 3ds at the folder : `/3ds/JerosSaveSync/JerosSaveSync.ini`
6. Launch `make` command into root of the project.
   1. Alternative : `APP_PUBLIC_KEY` and `APP_SECRET_KEY` can be passed by argument.
      1. ex : `make USER_FLAGS='-DAPP_PUBLIC_KEY=\"APP_PUBLIC_KEY\" -DAPP_SECRET_KEY=\"APP_SECRET_KEY\"'`
7. Transfert the generated file `output/3ds-arm/3ds/JerosSaveSync/JerosSaveSync.3dsx` into your 3ds
8. Launch it

To debug app

1. Delete `build` and `output` folder if exists
2. Launch `make` with the flag DEBUG : `make USER_FLAGS="-DDEBUG=1"`

## TODO ##

1. Add verification exists : by size and last updated date before upload/download. If same distant and local, skip
2. Debug spacial name. Ex : `0x01CA8 LAYTON'S MYSTERY JOURNEY™` or `0x01D15 Mario & Luigi  Bowser's Inside…` with `™` and `…`