# Contributing

`abb-rws` is a hard fork of [ros-industrial/abb_librws](https://github.com/ros-industrial/abb_librws). Bugs and changes specific to this fork (RWS 2.0 support, the `abb_rws` package name, conda-forge packaging) belong in the [issue tracker of this repository](https://github.com/Carlo-Rosso/abb-rws/issues), not upstream.

1. Open an issue describing the problem or the change before starting larger work.
2. Fork the repository, create a feature branch and open a pull request.
3. Check that the project builds before opening the pull request:

   ```sh
   pixi run build             # library + example, dependencies from conda-forge
   pixi run -e ros colcon     # package.xml validation + colcon build
   ```

4. Keep the existing copyright headers in the source files.

New contributors are listed in [AUTHORS.md](AUTHORS.md).

## Releasing

The version is written in one place: `<version>` in [package.xml](package.xml). CMake reads it (`project()` version, `abb_rwsConfigVersion.cmake`, library `SOVERSION` = major version), and the ROS/RoboStack tooling reads `package.xml` directly.

1. Update `<version>` in `package.xml` (semantic versioning: bump the major version for breaking API/ABI changes).
2. Commit, then tag the commit with the same version prefixed by `v` and push the tag:

   ```sh
   git tag v1.2.0
   git push origin v1.2.0
   ```

   CI fails if the tag does not match `package.xml`.
3. Create the GitHub release from the tag.
4. Update the conda-forge recipe (`version` and the `sha256` of the release archive `https://github.com/Carlo-Rosso/abb-rws/archive/refs/tags/v<version>.tar.gz`). After the feedstock exists, the conda-forge bot opens this pull request automatically for every new GitHub release.
