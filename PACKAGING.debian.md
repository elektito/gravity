Debian Packaging Guide
======================

1. Create a clean clone of the source tree.

        cd /tmp/
        git clone ~/source/gravity/

2. Remove the .git directory.

        rm -rf gravity/.git

3. Unpack `waf` as described
   [here](https://wiki.debian.org/UnpackWaf).

        cd gravity/
        ./waf --help &> /dev/null
        mv .waf-*/* .
        sed -i '/^#==>$/,$d' waf
        rmdir .waf-*
        cd ..

4. Rename the source directory to gravity-X.Y in which X.Y is the
   version number.

        mv gravity gravity-1.0

5. Create a source tarball.

        tar -czf gravity_1.0.orig.tar.gz

6. Build the Debian package. Make sure you have the proper tools
   (`sudo apt-get install devscripts`).

        cd gravity-1.0/
        debuild -us -uc
