### Imports

import glob, shutil, os, sys, argparse, subprocess


### Constants

# List of available platforms
platformlist = [
    'vs2008',
    'tutovs2008',
    'vs2010',
    'tutovs2010',
    'vs2012',
    'tutovs2012',
    'vs2017-32',
    'tutovs2017-32',
    'vs2017-64',
    'tutovs2017-64',
    'vs2019-32',
    'tutovs2019-32',
    'vs2019-64',
    'tutovs2019-64',
    'vs2022-32',
    'tutovs2022-32',
    'vs2022-64',
    'tutovs2022-64',
    'mingw-32',
    'mingw-64',
    'tutomingw-32',
    'tutomingw-64',
    'mac',
    'tutomac',
    'linux32',
    'tutolinux32',
    'linux64',
    'tutolinux64',
    'ios',
    'android',
    'android-native',
    'doxygen',
    'src'
]

# Base source path
basesrc = '../..'

# Work directory
workdir = 'workdir'


### Processes command line

# Gets parameters from command line arguments
if __name__ == '__main__':
    parser      = argparse.ArgumentParser()
    parser.add_argument('-d', '--date', default = '', nargs='?')
    parser.add_argument('-p', '--platform', default = 'doxygen', choices = platformlist)
    args        = parser.parse_args()
    platform    = args.platform
    version     = subprocess.check_output(os.path.abspath('../../bin/orx') + ' -v true', shell = True).rstrip().decode("utf-8")
    print('Version: ' + version)
    print('Platform: ' + platform)
    if args.date != '' and args.date != None:
      print('Date: ' + args.date)
      version += '-' + args.date


### Variables

# List of file info
basefileinfolist = [
    {'src': '../AUTHORS',                                               'dst': None},
    {'src': '../CHANGELOG',                                             'dst': None},
    {'src': '../COPYRIGHT',                                             'dst': None},
    {'src': '../LICENSE',                                               'dst': None},
    {'src': '../README.md',                                             'dst': None},
    {'src': '../README.html',                                           'dst': None}
]

docfileinfolist = basefileinfolist

srcfileinfolist = basefileinfolist + [
    {'src': 'bin/*.ini',                                                'dst': None},
    {'src': 'build/premake4.lua',                                       'dst': None},
    {'src': 'build/orx.*',                                              'dst': None},
    {'src': '../.extern',                                               'dst': None},
    {'src': '../setup*',                                                'dst': None},
    {'src': '../init*',                                                 'dst': None}
]

devfileinfolist = basefileinfolist + [
    {'src': 'bin/*Template.ini',                                        'dst': '.'}
]

vsfileinfolist = devfileinfolist + [
    {'src': 'lib/dynamic/orx*.dll',                                     'dst': 'lib'},
    {'src': 'lib/dynamic/orx*.lib',                                     'dst': 'lib'},
    {'src': 'lib/dynamic/orx*.pdb',                                     'dst': 'lib'}
]

tutorialvsfileinfolist = [
    {'src': 'lib/dynamic/orx*.lib',                                     'dst': 'lib'},
    {'src': 'lib/dynamic/orx*.pdb',                                     'dst': 'lib'},
    {'src': '../tutorial/build/premake4*',                              'dst': 'build'},
    {'src': '../tutorial/build/windows/' + platform[4:10] + '/*proj*',  'dst': 'build/windows/' + platform[4:10]},
    {'src': '../tutorial/build/windows/' + platform[4:10] + '/*.sln',   'dst': 'build/windows/' + platform[4:10]}
]

mingwfileinfolist = devfileinfolist + [
    {'src': 'lib/dynamic/orx*.dll',                                     'dst': 'lib'},
    {'src': 'lib/dynamic/liborx*.a',                                    'dst': 'lib'}
]

tutorialmingwfileinfolist = [
    {'src': 'lib/dynamic/liborx*.a',                                    'dst': 'lib'},
    {'src': '../tutorial/build/premake4*',                              'dst': 'build'},
    {'src': '../tutorial/build/windows/codeblocks/*.cbp',               'dst': 'build/windows/codeblocks'},
    {'src': '../tutorial/build/windows/codeblocks/*.workspace',         'dst': 'build/windows/codeblocks'},
    {'src': '../tutorial/build/windows/codelite/*.project',             'dst': 'build/windows/codelite'},
    {'src': '../tutorial/build/windows/codelite/*.workspace',           'dst': 'build/windows/codelite'},
    {'src': '../tutorial/build/windows/gmake/*.make',                   'dst': 'build/windows/gmake'},
    {'src': '../tutorial/build/windows/gmake/Makefile',                 'dst': 'build/windows/gmake'}
]

macfileinfolist = devfileinfolist + [
    {'src': 'lib/dynamic/liborx*.dylib',                                'dst': 'lib'}
]

tutorialmacfileinfolist = [
    {'src': 'lib/dynamic/liborx*.dylib',                                'dst': 'lib'},
    {'src': '../tutorial/build/premake4*',                              'dst': 'build'},
    {'src': '../tutorial/build/mac/codeblocks/*.cbp',                   'dst': 'build/mac/codeblocks'},
    {'src': '../tutorial/build/mac/codeblocks/*.workspace',             'dst': 'build/mac/codeblocks'},
    {'src': '../tutorial/build/mac/codelite/*.project',                 'dst': 'build/mac/codelite'},
    {'src': '../tutorial/build/mac/codelite/*.workspace',               'dst': 'build/mac/codelite'},
    {'src': '../tutorial/build/mac/gmake/*.make',                       'dst': 'build/mac/gmake'},
    {'src': '../tutorial/build/mac/gmake/Makefile',                     'dst': 'build/mac/gmake'},
    {'src': '../tutorial/build/mac/xcode4/Tutorial.xcodeproj/*.pbxproj','dst': 'build/mac/xcode4/Tutorial.xcodeproj'}
]

linuxfileinfolist = devfileinfolist + [
    {'src': 'lib/dynamic/liborx*.so',                                   'dst': 'lib'}
]

tutoriallinuxfileinfolist = [
    {'src': 'lib/dynamic/liborx*.so',                                   'dst': 'lib'},
    {'src': '../tutorial/build/premake4*',                              'dst': 'build'},
    {'src': '../tutorial/build/linux/codeblocks/*.cbp',                 'dst': 'build/linux/codeblocks'},
    {'src': '../tutorial/build/linux/codeblocks/*.workspace',           'dst': 'build/linux/codeblocks'},
    {'src': '../tutorial/build/linux/codelite/*.project',               'dst': 'build/linux/codelite'},
    {'src': '../tutorial/build/linux/codelite/*.workspace',             'dst': 'build/linux/codelite'},
    {'src': '../tutorial/build/linux/gmake/*.make',                     'dst': 'build/linux/gmake'},
    {'src': '../tutorial/build/linux/gmake/Makefile',                   'dst': 'build/linux/gmake'}
]

iosfileinfolist = devfileinfolist + [
    {'src': '../extern/LiquidFun-1.1.0/src/liquidfun/Box2D/License.txt','dst': None},
    {'src': '../liborx-fat-*',                                          'dst': None},
    {'src': 'build/ios/xcode/Info.plist',                               'dst': None},
    {'src': 'demo/iOS/ReadMe-iOS.html',                                 'dst': '..'}
]

androidfileinfolist = devfileinfolist + [
    {'src': 'build/android/*.sh',                                       'dst': None},
    {'src': 'build/android/*.properties',                               'dst': None},
    {'src': 'build/android/obj/local/armeabi-v7a/liborx*.a',            'dst': 'lib/static/android/armeabi-v7a'},
    {'src': 'build/android/obj/local/arm64-v8a/liborx*.a',              'dst': 'lib/static/android/arm64-v8a'},
    {'src': 'build/android/obj/local/x86/liborx*.a',                    'dst': 'lib/static/android/x86'}
]

androidnativefileinfolist = devfileinfolist + [
    {'src': 'build/android-native/*.sh',                                'dst': None},
    {'src': 'build/android-native/*.properties',                        'dst': None},
    {'src': 'build/android-native/obj/local/armeabi-v7a/liborx*.a',     'dst': 'lib/static/android-native/armeabi-v7a'},
    {'src': 'build/android-native/obj/local/arm64-v8a/liborx*.a',       'dst': 'lib/static/android-native/arm64-v8a'},
    {'src': 'build/android-native/obj/local/x86/liborx*.a',             'dst': 'lib/static/android-native/x86'}
]

# List of folder info
docfolderinfolist = [
    {'src': '../doc/html',                                              'dst': None}
]

devfolderinfolist = [
    {'src': 'include',                                                  'dst': None},
    {'src': '../tools/orxCrypt/bin',                                    'dst': 'tools/orxCrypt/bin'},
    {'src': '../tools/orxFontGen/bin',                                  'dst': 'tools/orxFontGen/bin'},
]

tutorialfolderinfolist = [
    {'src': 'include',                                                  'dst': None},
    {'src': '../tutorial/bin',                                          'dst': 'bin'},
    {'src': '../tutorial/data',                                         'dst': 'data'},
    {'src': '../tutorial/src',                                          'dst': 'src'}
]

iosfolderinfolist = docfolderinfolist + [
    {'src': 'build/ios/xcode/orx-ios.xcodeproj',                        'dst': None},
    {'src': 'demo/iOS',                                                 'dst': None},
    {'src': '../extern/libwebp/include',                                'dst': None},
    {'src': '../extern/libwebp/lib/ios',                                'dst': None},
    {'src': '../extern/LiquidFun-1.1.0/src/liquidfun/Box2D/build/Xcode/Box2D-ios.xcodeproj','dst': None},
    {'src': '../extern/LiquidFun-1.1.0/include',                        'dst': None},
    {'src': '../extern/LiquidFun-1.1.0/lib/ios',                        'dst': None},
    {'src': '../extern/LiquidFun-1.1.0/src/liquidfun/Box2D/Box2D',      'dst': None},
    {'src': '../extern/miniaudio',                                      'dst': None},
    {'src': '../extern/qoi',                                            'dst': None},
    {'src': '../extern/rpmalloc/rpmalloc',                              'dst': None},
    {'src': '../extern/stb_image',                                      'dst': None},
    {'src': '../extern/stb_vorbis',                                     'dst': None},
    {'src': '../extern/xxHash',                                         'dst': None},
    {'src': 'plugins/Display/iOS',                                      'dst': None},
    {'src': 'plugins/Joystick/iOS',                                     'dst': None},
    {'src': 'plugins/Keyboard/Dummy',                                   'dst': None},
    {'src': 'plugins/Mouse/iOS',                                        'dst': None},
    {'src': 'plugins/Physics/LiquidFun',                                'dst': None},
    {'src': 'plugins/Render/Home',                                      'dst': None},
    {'src': 'plugins/Sound/iOS',                                        'dst': None},
    {'src': 'include',                                                  'dst': None},
    {'src': 'src',                                                      'dst': None}
]

androidfolderinfolist = devfolderinfolist + [
    {'src': 'build/android/jni',                                        'dst': None},
    {'src': 'lib/static/android',                                       'dst': None},
    {'src': '../extern/android',                                        'dst': None},
    {'src': '../extern/LiquidFun-1.1.0/lib/android',                    'dst': None},
    {'src': '../extern/libwebp/lib/android',                            'dst': None},
    {'src': 'demo/android',                                             'dst': None},
    {'src': 'plugins/Display/android',                                  'dst': None},
    {'src': 'plugins/Joystick/android',                                 'dst': None},
    {'src': 'plugins/Keyboard/android',                                 'dst': None},
    {'src': 'plugins/Mouse/android',                                    'dst': None},
    {'src': 'plugins/Physics/LiquidFun',                                'dst': None},
    {'src': 'plugins/Render/Home',                                      'dst': None},
    {'src': 'plugins/Sound/MiniAudio',                                  'dst': None}
]

androidnativefolderinfolist = devfolderinfolist + [
    {'src': 'build/android-native/jni',                                 'dst': None},
    {'src': 'lib/static/android-native',                                'dst': None},
    {'src': '../extern/LiquidFun-1.1.0/lib/android',                    'dst': None},
    {'src': '../extern/libwebp/lib/android',                            'dst': None},
    {'src': 'demo/android-native',                                      'dst': None},
    {'src': 'plugins/Display/android',                                  'dst': None},
    {'src': 'plugins/Joystick/android',                                 'dst': None},
    {'src': 'plugins/Keyboard/android',                                 'dst': None},
    {'src': 'plugins/Mouse/android',                                    'dst': None},
    {'src': 'plugins/Physics/LiquidFun',                                'dst': None},
    {'src': 'plugins/Render/Home',                                      'dst': None},
    {'src': 'plugins/Sound/MiniAudio',                                  'dst': None}
]

srcfolderinfolist = [
    {'src': '../tools',                                                 'dst': None},
    {'src': 'bin/data',                                                 'dst': None},
    {'src': 'bin/plugins',                                              'dst': None},
    {'src': 'build/android',                                            'dst': None},
    {'src': 'build/android-native',                                     'dst': None},
    {'src': 'build/ios',                                                'dst': None},
    {'src': 'build/rebol',                                              'dst': None},
    {'src': 'build/template',                                           'dst': None},
    {'src': 'include',                                                  'dst': None},
    {'src': 'plugins',                                                  'dst': None},
    {'src': 'src',                                                      'dst': None}
]


### Platform info

platforminfolist = {

    'vs2008': {
        'foldername':       'dev-vs2008',
        'filename':         'dev-vs2008',
        'format':           'zip',
        'fileinfolist':     vsfileinfolist,
        'folderinfolist':   devfolderinfolist
    },

    'tutovs2008': {
        'foldername':       'tutorial-vs2008',
        'filename':         'tutorial-vs2008',
        'format':           'zip',
        'fileinfolist':     tutorialvsfileinfolist,
        'folderinfolist':   tutorialfolderinfolist
    },

    'vs2010': {
        'foldername':       'dev-vs2010',
        'filename':         'dev-vs2010',
        'format':           'zip',
        'fileinfolist':     vsfileinfolist,
        'folderinfolist':   devfolderinfolist
    },

    'tutovs2010': {
        'foldername':       'tutorial-vs2010',
        'filename':         'tutorial-vs2010',
        'format':           'zip',
        'fileinfolist':     tutorialvsfileinfolist,
        'folderinfolist':   tutorialfolderinfolist
    },

    'vs2012': {
        'foldername':       'dev-vs2012',
        'filename':         'dev-vs2012',
        'format':           'zip',
        'fileinfolist':     vsfileinfolist,
        'folderinfolist':   devfolderinfolist
    },

    'tutovs2012': {
        'foldername':       'tutorial-vs2012',
        'filename':         'tutorial-vs2012',
        'format':           'zip',
        'fileinfolist':     tutorialvsfileinfolist,
        'folderinfolist':   tutorialfolderinfolist
    },

    'vs2017-32': {
        'foldername':       'dev-vs2017-32',
        'filename':         'dev-vs2017-32',
        'format':           'zip',
        'fileinfolist':     vsfileinfolist,
        'folderinfolist':   devfolderinfolist
    },

    'vs2017-64': {
        'foldername':       'dev-vs2017-64',
        'filename':         'dev-vs2017-64',
        'format':           'zip',
        'fileinfolist':     vsfileinfolist,
        'folderinfolist':   devfolderinfolist
    },

    'tutovs2017-32': {
        'foldername':       'tutorial-vs2017-32',
        'filename':         'tutorial-vs2017-32',
        'format':           'zip',
        'fileinfolist':     tutorialvsfileinfolist,
        'folderinfolist':   tutorialfolderinfolist
    },

    'tutovs2017-64': {
        'foldername':       'tutorial-vs2017-64',
        'filename':         'tutorial-vs2017-64',
        'format':           'zip',
        'fileinfolist':     tutorialvsfileinfolist,
        'folderinfolist':   tutorialfolderinfolist
    },

    'vs2019-32': {
        'foldername':       'dev-vs2019-32',
        'filename':         'dev-vs2019-32',
        'format':           'zip',
        'fileinfolist':     vsfileinfolist,
        'folderinfolist':   devfolderinfolist
    },

    'vs2019-64': {
        'foldername':       'dev-vs2019-64',
        'filename':         'dev-vs2019-64',
        'format':           'zip',
        'fileinfolist':     vsfileinfolist,
        'folderinfolist':   devfolderinfolist
    },

    'tutovs2019-32': {
        'foldername':       'tutorial-vs2019-32',
        'filename':         'tutorial-vs2019-32',
        'format':           'zip',
        'fileinfolist':     tutorialvsfileinfolist,
        'folderinfolist':   tutorialfolderinfolist
    },

    'tutovs2019-64': {
        'foldername':       'tutorial-vs2019-64',
        'filename':         'tutorial-vs2019-64',
        'format':           'zip',
        'fileinfolist':     tutorialvsfileinfolist,
        'folderinfolist':   tutorialfolderinfolist
    },

    'vs2022-32': {
        'foldername':       'dev-vs2022-32',
        'filename':         'dev-vs2022-32',
        'format':           'zip',
        'fileinfolist':     vsfileinfolist,
        'folderinfolist':   devfolderinfolist
    },

    'vs2022-64': {
        'foldername':       'dev-vs2022-64',
        'filename':         'dev-vs2022-64',
        'format':           'zip',
        'fileinfolist':     vsfileinfolist,
        'folderinfolist':   devfolderinfolist
    },

    'tutovs2022-32': {
        'foldername':       'tutorial-vs2022-32',
        'filename':         'tutorial-vs2022-32',
        'format':           'zip',
        'fileinfolist':     tutorialvsfileinfolist,
        'folderinfolist':   tutorialfolderinfolist
    },

    'tutovs2022-64': {
        'foldername':       'tutorial-vs2022-64',
        'filename':         'tutorial-vs2022-64',
        'format':           'zip',
        'fileinfolist':     tutorialvsfileinfolist,
        'folderinfolist':   tutorialfolderinfolist
    },

    'mingw-32': {
        'foldername':       'dev-mingw-32',
        'filename':         'dev-mingw-32',
        'format':           'zip',
        'fileinfolist':     mingwfileinfolist,
        'folderinfolist':   devfolderinfolist
    },

    'mingw-64': {
        'foldername':       'dev-mingw-64',
        'filename':         'dev-mingw-64',
        'format':           'zip',
        'fileinfolist':     mingwfileinfolist,
        'folderinfolist':   devfolderinfolist
    },

    'tutomingw-32': {
        'foldername':       'tutorial-mingw-32',
        'filename':         'tutorial-mingw-32',
        'format':           'zip',
        'fileinfolist':     tutorialmingwfileinfolist,
        'folderinfolist':   tutorialfolderinfolist
    },

    'tutomingw-64': {
        'foldername':       'tutorial-mingw-64',
        'filename':         'tutorial-mingw-64',
        'format':           'zip',
        'fileinfolist':     tutorialmingwfileinfolist,
        'folderinfolist':   tutorialfolderinfolist
    },

    'mac': {
        'foldername':       'dev-mac',
        'filename':         'dev-mac',
        'format':           'zip',
        'fileinfolist':     macfileinfolist,
        'folderinfolist':   devfolderinfolist
    },

    'tutomac': {
        'foldername':       'tutorial-mac',
        'filename':         'tutorial-mac',
        'format':           'zip',
        'fileinfolist':     tutorialmacfileinfolist,
        'folderinfolist':   tutorialfolderinfolist
    },

    'linux32': {
        'foldername':       'dev-linux32',
        'filename':         'dev-linux32',
        'format':           'bztar',
        'fileinfolist':     linuxfileinfolist,
        'folderinfolist':   devfolderinfolist
    },

    'tutolinux32': {
        'foldername':       'tutorial-linux32',
        'filename':         'tutorial-linux32',
        'format':           'bztar',
        'fileinfolist':     tutoriallinuxfileinfolist,
        'folderinfolist':   tutorialfolderinfolist
    },

    'linux64': {
        'foldername':       'dev-linux64',
        'filename':         'dev-linux64',
        'format':           'bztar',
        'fileinfolist':     linuxfileinfolist,
        'folderinfolist':   devfolderinfolist
    },

    'tutolinux64': {
        'foldername':       'tutorial-linux64',
        'filename':         'tutorial-linux64',
        'format':           'bztar',
        'fileinfolist':     tutoriallinuxfileinfolist,
        'folderinfolist':   tutorialfolderinfolist
    },

    'ios': {
        'foldername':       'dev-ios',
        'filename':         'full-ios',
        'format':           'zip',
        'fileinfolist':     iosfileinfolist,
        'folderinfolist':   iosfolderinfolist
    },

    'android': {
        'foldername':       'dev-android',
        'filename':         'dev-android',
        'format':           'zip',
        'fileinfolist':     androidfileinfolist,
        'folderinfolist':   androidfolderinfolist
    },

    'android-native': {
        'foldername':       'dev-android-native',
        'filename':         'dev-android-native',
        'format':           'zip',
        'fileinfolist':     androidnativefileinfolist,
        'folderinfolist':   androidnativefolderinfolist
    },

    'doxygen': {
        'foldername':       'doc',
        'filename':         'doc',
        'format':           'zip',
        'fileinfolist':     docfileinfolist,
        'folderinfolist':   docfolderinfolist
    },

    'src': {
        'foldername':       'code',
        'filename':         'src',
        'format':           'zip',
        'fileinfolist':     srcfileinfolist,
        'folderinfolist':   srcfolderinfolist
    }
}


### Prepares

# Deletes packing folder if present
if os.path.exists(workdir):
    shutil.rmtree(workdir)

# Gets base destination path
basedst = os.path.join(workdir, os.path.join('orx-' + version, platforminfolist[platform]['foldername']))


### Copies folders

# Should copy folders?
if platforminfolist[platform]['folderinfolist']:

    # Creates destination path if needed
    if not os.path.exists(basedst):
        os.makedirs(basedst)

    # For all folder info for this platform
    for folderinfo in platforminfolist[platform]['folderinfolist']:

        # Gets source
        source = os.path.join(basesrc, folderinfo['src'])

        # Exists?
        if os.path.exists(source):

            # Gets destination
            if folderinfo['dst']:
                destination = os.path.join(basedst, folderinfo['dst'])
            else:
                destination = os.path.join(basedst, folderinfo['src'])

            # Does the destination folder exist?
            if os.path.exists(destination):

                # Deletes it
                shutil.rmtree(destination)

            # Logs
            print('Copying folder: ' + folderinfo['src'])

            # Copies folder
            shutil.copytree(source, destination)

        else:

            # Logs
            print('Skipping folder: ' + folderinfo['src'])


### Copies files

# Should copy files?
if platforminfolist[platform]['fileinfolist']:

    # Creates destination path if needed
    if not os.path.exists(basedst):
        os.makedirs(basedst)

    # For all file info for this platform
    for fileinfo in platforminfolist[platform]['fileinfolist']:

        # For all files
        for file in glob.iglob(os.path.join(basesrc, fileinfo['src'])):

            # Gets destination
            if fileinfo['dst']:
                destination = os.path.join(basedst, fileinfo['dst'])
            else:
                destination = os.path.join(basedst, os.path.dirname(os.path.relpath(file, basesrc)))

            # Logs
            print('Copying file: ' + os.path.basename(file))

            # Creates path if needed
            if not os.path.exists(destination):
                os.makedirs(destination)

            # Copies file
            shutil.copy2(file, destination)


### Exports archive

# Creates archive from folder
archive = shutil.make_archive(os.path.join('packages', 'orx-' + platforminfolist[platform]['filename'] + '-' + version), platforminfolist[platform]['format'], workdir, '.')

# Logs
print('Created archive: ' + os.path.basename(archive))
