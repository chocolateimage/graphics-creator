# Releasing

1. Change version number in `src/shared.hpp`, `scripts/winsetup.iss`
2. Add release to metainfo, and commit
3. On Windows (msys ucrt64): Update packages, git pull, `./scripts/windeploy.sh`
4. Create release (not mark as latest) with the template below
5. Update Flatpak manifest for new version and tag
6. Build Flatpak locally and test: `flatpak-builder --force-clean --user --install-deps-from=flathub --repo=repo --install builddir me.chocolateimage.graphics-creator.yml`
7. Commit Flatpak manifest
8. SSH to server
9. `rm me.chocolateimage.graphics-creator.yml`
10. `wget https://github.com/chocolateimage/graphics-creator/raw/refs/heads/main/me.chocolateimage.graphics-creator.yml`
11. `./build.sh`
12. Upload .exe
13. Zip sample-plugin directory itself and upload that with the filename `sample-plugin-v0.5.0.zip`
14. Set GitHub release to latest

Template for GitHub release (change version):

```md
> [!WARNING]
> Undo/redo hasn't been fully implemented. Do not rely on it too heavily.

## Download

- [Windows setup download](https://github.com/chocolateimage/graphics-creator/releases/download/v0.5.0/graphics-creator-setup-v0.5.0.exe)
- [Linux Flatpak install](https://flatpak.chocolateimage.me/me.chocolateimage.graphics-creator.flatpakref)

The Windows download is also available at the bottom in the assets menu. The Linux Flatpak is a reference to the repository which is hosting the Flatpak, allowing for updates with `flatpak update`.

## New in this release

- something


## Smaller changes

- something


**Full Changelog**: https://github.com/chocolateimage/graphics-creator/compare/v0.4.0...v0.5.0
```
