### Imports

import glob, shutil, os, sys, argparse


### Constants

# List of available platforms
platformlist    = ['vs2008', 'vs2010', 'mingw', 'mac', 'linux32', 'linux64', 'android', 'android-native', 'doxygen', 'src', 'extern']

# Base source path
basesrc         = '../..'

# Work directory
workdir         = 'workdir'


### Processes command line

# Gets parameters from command line arguments
if __name__ == '__main__':
    parser      = argparse.ArgumentParser()
    parser.add_argument('-v', '--version', default = 'latest')
    parser.add_argument('-p', '--platform', default = 'doxygen', choices = platformlist)
    args        = parser.parse_args()
    version     = args.version
    platform    = args.platform

    # Logs
    print('Version: ' + version)
    print('Platform: ' + platform)


### Variables

# List of file info
basefileinfolist = [
    {'src': '../AUTHORS',                                               'dst': None},
    {'src': '../CHANGELOG',                                             'dst': None},
    {'src': '../COPYRIGHT',                                             'dst': None},
    {'src': '../LICENSE',                                               'dst': None},
    {'src': '../README',                                                'dst': None}
]

docfileinfolist = basefileinfolist

codefileinfolist = basefileinfolist + [
    {'src': 'build/premake4.lua',                                       'dst': None},
]

externfileinfolist = basefileinfolist

devfileinfolist = basefileinfolist + [
    {'src': 'bin/*Template.ini',                                        'dst': '.'}
]

vsfileinfolist = devfileinfolist + [
    {'src': 'lib/dynamic/orx*.dll',                                     'dst': 'lib'},
    {'src': 'lib/dynamic/orx*.lib',                                     'dst': 'lib'},
    {'src': 'lib/dynamic/orx*.pdb',                                     'dst': 'lib'}
]

mingwfileinfolist = devfileinfolist + [
    {'src': 'lib/dynamic/orx*.dll',                                     'dst': 'lib'},
    {'src': 'lib/dynamic/liborx*.a',                                    'dst': 'lib'}
]

macfileinfolist = devfileinfolist + [
    {'src': 'lib/dynamic/liborx*.dylib',                                'dst': 'lib'}
]

linuxfileinfolist = devfileinfolist + [
    {'src': 'lib/dynamic/liborx*.so',                                   'dst': 'lib'}
]

androidfileinfolist = devfileinfolist + [
    {'src': 'build/android/obj/local/armaebi/liborx*.a',                'dst': 'lib/static/android/armaebi'},
    {'src': 'build/android/obj/local/armaebi-v7a/liborx*.a',            'dst': 'lib/static/android/armaebi-v7a'}
]

androidnativefileinfolist = devfileinfolist + [
    {'src': 'build/android-native/obj/local/armaebi/liborx*.a',         'dst': 'lib/static/android-native/armaebi'},
    {'src': 'build/android-native/obj/local/armaebi-v7a/liborx*.a',     'dst': 'lib/static/android-native/armaebi-v7a'}
]

# List of folder info
docfolderinfolist = [
    {'src': '../doc/html',                                              'dst': None}
]

devfolderinfolist = [
    {'src': 'include',                                                  'dst': None}
]

externfolderinfolist = [
    {'src': '../extern/Box2D_2.1.3',                                    'dst': None},
    {'src': '../extern/dlmalloc',                                       'dst': None},
    {'src': '../extern/glfw-2.7',                                       'dst': None},
    {'src': '../extern/libsndfile-1.0.22',                              'dst': None},
    {'src': '../extern/openal-soft',                                    'dst': None},
    {'src': '../extern/SOIL',                                           'dst': None},
    {'src': '../extern/stb_vorbis',                                     'dst': None}
]

androidfolderinfolist = devfolderinfolist + [
    {'src': '../extern/android',                                        'dst': None},
    {'src': '../extern/Box2D_2.1.3/lib/android',                        'dst': None},
    {'src': '../extern/SOIL/lib/android',                               'dst': None}
]

androidnativefolderinfolist = devfolderinfolist + [
    {'src': '../extern/nv_and_util',                                    'dst': None},
    {'src': '../extern/Box2D_2.1.3/lib/android-native',                 'dst': None},
    {'src': '../extern/SOIL/lib/android-native',                        'dst': None}
]

codefolderinfolist = [
    {'src': 'bin',                                                      'dst': None},
    {'src': 'build/linux',                                              'dst': None},
    {'src': 'build/mac/xcode/orx-embedded.xcodeproj',                   'dst': None},
    {'src': 'build/mac/codeblocks',                                     'dst': None},
    {'src': 'build/mac/codelite',                                       'dst': None},
    {'src': 'build/mac/gmake',                                          'dst': None},
    {'src': 'build/windows/vs2008',                                     'dst': None},
    {'src': 'build/windows/vs2010',                                     'dst': None},
    {'src': 'build/windows/codeblocks',                                 'dst': None},
    {'src': 'build/windows/codelite',                                   'dst': None},
    {'src': 'build/windows/gmake',                                      'dst': None},
    {'src': 'demo',                                                     'dst': None},
    {'src': 'include',                                                  'dst': None},
    {'src': 'lib',                                                      'dst': None},
    {'src': 'plugins/Demo',                                             'dst': None},
    {'src': 'plugins/Display/GLFW',                                     'dst': None},
    {'src': 'plugins/Joystick/GLFW',                                    'dst': None},
    {'src': 'plugins/Keyboard/GLFW',                                    'dst': None},
    {'src': 'plugins/Mouse/GLFW',                                       'dst': None},
    {'src': 'plugins/Physics/Box2D',                                    'dst': None},
    {'src': 'plugins/Render/Home',                                      'dst': None},
    {'src': 'plugins/Sound/OpenAL',                                     'dst': None},
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

    'vs2010': {
        'foldername':       'dev-vs2010',
        'filename':         'dev-vs2010',
        'format':           'zip',
        'fileinfolist':     vsfileinfolist,
        'folderinfolist':   devfolderinfolist
    },

    'mingw': {
        'foldername':       'dev-mingw',
        'filename':         'dev-mingw',
        'format':           'zip',
        'fileinfolist':     mingwfileinfolist,
        'folderinfolist':   devfolderinfolist
    },

    'mac': {
        'foldername':       'dev-mac',
        'filename':         'dev-mac',
        'format':           'zip',
        'fileinfolist':     macfileinfolist,
        'folderinfolist':   devfolderinfolist
    },

    'linux32': {
        'foldername':       'dev-linux32',
        'filename':         'dev-linux32',
        'format':           'bztar',
        'fileinfolist':     linuxfileinfolist,
        'folderinfolist':   devfolderinfolist
    },

    'linux64': {
        'foldername':       'dev-linux64',
        'filename':         'dev-linux64',
        'format':           'bztar',
        'fileinfolist':     linuxfileinfolist,
        'folderinfolist':   devfolderinfolist
    },

    'android': {
        'foldername':       'dev-android',
        'filename':         'full-android',
        'format':           'zip',
        'fileinfolist':     androidfileinfolist,
        'folderinfolist':   androidfolderinfolist
    },

    'android-native': {
        'foldername':       'dev-android-native',
        'filename':         'full-android-native',
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
        'fileinfolist':     codefileinfolist,
        'folderinfolist':   codefolderinfolist
    },

    'extern': {
        'foldername':       'extern',
        'filename':         'extern',
        'format':           'zip',
        'fileinfolist':     externfileinfolist,
        'folderinfolist':   externfolderinfolist
    },
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
