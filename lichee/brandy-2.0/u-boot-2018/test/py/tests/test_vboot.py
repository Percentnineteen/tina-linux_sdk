# SPDX-License-Identifier:	GPL-2.0+
# Copyright (c) 2016, Google Inc.
#
# U-Boot Verified Boot Test

"""
This tests verified boot in the following ways:

For image verification:
- Create FIT (unsigned) with mkimage
- Check that verification shows that no keys are verified
- Sign image
- Check that verification shows that a key is now verified

For configuration verification:
- Corrupt signature and check for failure
- Create FIT (with unsigned configuration) with mkimage
- Check that image verification works
- Sign the FIT and mark the key as 'required' for verification
- Check that image verification works
- Corrupt the signature
- Check that image verification no-longer works

Tests run with both SHA1 and SHA256 hashing.
"""

import shutil
import struct
import pytest
import sys
import u_boot_utils as util
import vboot_forge
import vboot_evil

# Only run the full suite on a few combinations, since it doesn't add any more
# test coverage.
TESTDATA = [
    ['sha1', '', None, False, True],
    ['sha1', '', '-E -p 0x10000', False, False],
    ['sha1', '-pss', None, False, False],
    ['sha1', '-pss', '-E -p 0x10000', False, False],
    ['sha256', '', None, False, False],
    ['sha256', '', '-E -p 0x10000', False, False],
    ['sha256', '-pss', None, False, False],
    ['sha256', '-pss', '-E -p 0x10000', False, False],
    ['sha256', '-pss', None, True, False],
    ['sha256', '-pss', '-E -p 0x10000', True, True],
]

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('fit_signature')
@pytest.mark.requiredtool('dtc')
@pytest.mark.requiredtool('fdtget')
@pytest.mark.requiredtool('fdtput')
@pytest.mark.requiredtool('openssl')
def test_vboot(u_boot_console):
@pytest.mark.parametrize("sha_algo,padding,sign_options,required,full_test",
                         TESTDATA)
def test_vboot(u_boot_console, sha_algo, padding, sign_options, required,
               full_test):
    """Test verified boot signing with mkimage and verification with 'bootm'.

    This works using sandbox only as it needs to update the device tree used
    by U-Boot to hold public keys from the signing process.

    The SHA1 and SHA256 tests are combined into a single test since the
    key-generation process is quite slow and we want to avoid doing it twice.
    """
    def dtc(dts):
        """Run the device tree compiler to compile a .dts file

        The output file will be the same as the input file but with a .dtb
        extension.

        Args:
            dts: Device tree file to compile.
        """
        dtb = dts.replace('.dts', '.dtb')
        util.run_and_log(cons, 'dtc %s %s%s -O dtb '
                         '-o %s%s' % (dtc_args, datadir, dts, tmpdir, dtb))

    def run_bootm(sha_algo, test_type, expect_string, boots, fit=None):
        """Run a 'bootm' command U-Boot.

        This always starts a fresh U-Boot instance since the device tree may
        contain a new public key.

        Args:
            test_type: A string identifying the test type.
            expect_string: A string which is expected in the output.
            sha_algo: Either 'sha1' or 'sha256', to select the algorithm to
                    use.
            boots: A boolean that is True if Linux should boot and False if
                    we are expected to not boot
            fit: FIT filename to load and verify
        """
        if not fit:
            fit = '%stest.fit' % tmpdir
        cons.restart_uboot()
        with cons.log.section('Verified boot %s %s' % (sha_algo, test_type)):
            output = cons.run_command_list(
                ['sb load hostfs - 100 %stest.fit' % tmpdir,
                'fdt addr 100',
                'bootm 100'])
        assert(expect_string in ''.join(output))
                ['host load hostfs - 100 %s' % fit,
                 'fdt addr 100',
                 'bootm 100'])
        assert expect_string in ''.join(output)
        if boots:
            assert('sandbox: continuing, as we cannot run' in ''.join(output))

    def make_fit(its):
        """Make a new FIT from the .its source file.

        This runs 'mkimage -f' to create a new FIT.

        Args:
            its: Filename containing .its source.
        """
        util.run_and_log(cons, [mkimage, '-D', dtc_args, '-f',
                                '%s%s' % (datadir, its), fit])

    def sign_fit(sha_algo):
        """Sign the FIT

        Signs the FIT and writes the signature into it. It also writes the
        public key into the dtb.

        Args:
            sha_algo: Either 'sha1' or 'sha256', to select the algorithm to
                    use.
        """
        cons.log.action('%s: Sign images' % sha_algo)
        util.run_and_log(cons, [mkimage, '-F', '-k', tmpdir, '-K', dtb,
                                '-r', fit])

    def test_with_algo(sha_algo):
        """Test verified boot with the given hash algorithm.

        This is the main part of the test code. The same procedure is followed
        for both hashing algorithms.

        Args:
            sha_algo: Either 'sha1' or 'sha256', to select the algorithm to
                    use.
        """
        # Compile our device tree files for kernel and U-Boot. These are
        # regenerated here since mkimage will modify them (by adding a
        # public key) below.
        dtc('sandbox-kernel.dts')
        dtc('sandbox-u-boot.dts')

        # Build the FIT, but don't sign anything yet
        cons.log.action('%s: Test FIT with signed images' % sha_algo)
        make_fit('sign-images-%s.its' % sha_algo)
        run_bootm(sha_algo, 'unsigned images', 'dev-', True)

        # Sign images with our dev keys
        sign_fit(sha_algo)
        run_bootm(sha_algo, 'signed images', 'dev+', True)

        # Create a fresh .dtb without the public keys
        dtc('sandbox-u-boot.dts')

        cons.log.action('%s: Test FIT with signed configuration' % sha_algo)
        make_fit('sign-configs-%s.its' % sha_algo)
        run_bootm(sha_algo, 'unsigned config', '%s+ OK' % sha_algo, True)

        # Sign images with our dev keys
        sign_fit(sha_algo)
        run_bootm(sha_algo, 'signed config', 'dev+', True)

        cons.log.action('%s: Check signed config on the host' % sha_algo)

        util.run_and_log(cons, [fit_check_sign, '-f', fit, '-k', tmpdir,
                                '-k', dtb])
        util.run_and_log(cons, [fit_check_sign, '-f', fit, '-k', dtb])

        if full_test:
            # Make sure that U-Boot checks that the config is in the list of
            # hashed nodes. If it isn't, a security bypass is possible.
            ffit = '%stest.forged.fit' % tmpdir
            shutil.copyfile(fit, ffit)
            with open(ffit, 'rb') as fd:
                root, strblock = vboot_forge.read_fdt(fd)
            root, strblock = vboot_forge.manipulate(root, strblock)
            with open(ffit, 'w+b') as fd:
                vboot_forge.write_fdt(root, strblock, fd)
            util.run_and_log_expect_exception(
                cons, [fit_check_sign, '-f', ffit, '-k', dtb],
                1, 'Failed to verify required signature')

            run_bootm(sha_algo, 'forged config', 'Bad Data Hash', False, ffit)

            # Try adding an evil root node. This should be detected.
            efit = '%stest.evilf.fit' % tmpdir
            shutil.copyfile(fit, efit)
            vboot_evil.add_evil_node(fit, efit, evil_kernel, 'fakeroot')

            util.run_and_log_expect_exception(
                cons, [fit_check_sign, '-f', efit, '-k', dtb],
                1, 'Failed to verify required signature')
            run_bootm(sha_algo, 'evil fakeroot', 'Bad FIT kernel image format',
                      False, efit)

            # Try adding an @ to the kernel node name. This should be detected.
            efit = '%stest.evilk.fit' % tmpdir
            shutil.copyfile(fit, efit)
            vboot_evil.add_evil_node(fit, efit, evil_kernel, 'kernel@')

            msg = 'Signature checking prevents use of unit addresses (@) in nodes'
            util.run_and_log_expect_exception(
                cons, [fit_check_sign, '-f', efit, '-k', dtb],
                1, msg)
            run_bootm(sha_algo, 'evil kernel@', msg, False, efit)

        # Create a new properly signed fit and replace header bytes
        make_fit('sign-configs-%s%s.its' % (sha_algo, padding))
        sign_fit(sha_algo, sign_options)
        bcfg = u_boot_console.config.buildconfig
        max_size = int(bcfg.get('config_fit_signature_max_size', 0x10000000), 0)
        existing_size = replace_fit_totalsize(max_size + 1)
        run_bootm(sha_algo, 'Signed config with bad hash', 'Bad Data Hash',
                  False)
        cons.log.action('%s: Check overflowed FIT header totalsize' % sha_algo)

        # Replace with existing header bytes
        replace_fit_totalsize(existing_size)
        run_bootm(sha_algo, 'signed config', 'dev+', True)
        cons.log.action('%s: Check default FIT header totalsize' % sha_algo)

        # Increment the first byte of the signature, which should cause failure
        sig = util.run_and_log(cons, 'fdtget -t bx %s %s value' %
                               (fit, sig_node))
        byte_list = sig.split()
        byte = int(byte_list[0], 16)
        byte_list[0] = '%x' % (byte + 1)
        sig = ' '.join(byte_list)
        util.run_and_log(cons, 'fdtput -t bx %s %s value %s' %
                         (fit, sig_node, sig))

        run_bootm(sha_algo, 'Signed config with bad hash', 'Bad Data Hash', False)

        cons.log.action('%s: Check bad config on the host' % sha_algo)
        util.run_and_log_expect_exception(cons, [fit_check_sign, '-f', fit,
                '-k', dtb], 1, 'Failed to verify required signature')

    cons = u_boot_console
    tmpdir = cons.config.result_dir + '/'
    tmp = tmpdir + 'vboot.tmp'
    datadir = cons.config.source_dir + '/test/py/tests/vboot/'
    fit = '%stest.fit' % tmpdir
    mkimage = cons.config.build_dir + '/tools/mkimage'
    fit_check_sign = cons.config.build_dir + '/tools/fit_check_sign'
    dtc_args = '-I dts -O dtb -i %s' % tmpdir
    dtb = '%ssandbox-u-boot.dtb' % tmpdir
    sig_node = '/configurations/conf@1/signature@1'

    # Create an RSA key pair
    public_exponent = 65537
    util.run_and_log(cons, 'openssl genpkey -algorithm RSA -out %sdev.key '
                     '-pkeyopt rsa_keygen_bits:2048 '
                     '-pkeyopt rsa_keygen_pubexp:%d '
                     '2>/dev/null'  % (tmpdir, public_exponent))

    # Create a certificate containing the public key
    util.run_and_log(cons, 'openssl req -batch -new -x509 -key %sdev.key -out '
                     '%sdev.crt' % (tmpdir, tmpdir))

    # Create a number kernel image with zeroes
    with open('%stest-kernel.bin' % tmpdir, 'w') as fd:
        fd.write(5000 * chr(0))
    with open('%stest-kernel.bin' % tmpdir, 'wb') as fd:
        fd.write(500 * b'\0')

    # Create a second kernel image with ones
    evil_kernel = '%stest-kernel1.bin' % tmpdir
    with open(evil_kernel, 'wb') as fd:
        fd.write(500 * b'\x01')

    try:
        # We need to use our own device tree file. Remember to restore it
        # afterwards.
        old_dtb = cons.config.dtb
        cons.config.dtb = dtb
        test_with_algo('sha1')
        test_with_algo('sha256')
    finally:
        # Go back to the original U-Boot with the correct dtb.
        cons.config.dtb = old_dtb
        cons.restart_uboot()
