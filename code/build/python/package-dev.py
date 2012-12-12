### Imports

import glob, shutil, os, sys, argparse


### Constants

# List of available platforms
platformlist    = ['vs2008', 'vs2010', 'mingw', 'mac', 'linux32', 'linux64', 'doxygen', 'src']

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
    {'src': '../AUTHORS',               'dst': None},
    {'src': '../CHANGELOG',             'dst': None},
    {'src': '../COPYRIGHT',             'dst': None},
    {'src': '../LICENSE',               'dst': None},
    {'src': '../README',                'dst': None}
]

codefileinfolist = basefileinfolist + [
    {'src': 'build/premake4.lua',       'dst': None},
]

devfileinfolist = basefileinfolist + [
    {'src': 'bin/*Template.ini',        'dst': '.'},
]

vsfileinfolist = devfileinfolist + [
    {'src': 'lib/dynamic/orx*.dll',     'dst': 'lib'},
    {'src': 'lib/dynamic/orx*.lib',     'dst': 'lib'},
    {'src': 'lib/dynamic/orx*.pdb',     'dst': 'lib'}
]

mingwfileinfolist = devfileinfolist + [
    {'src': 'lib/dynamic/orx*.dll',     'dst': 'lib'},
    {'src': 'lib/dynamic/liborx*.a',    'dst': 'lib'}
]

macfileinfolist = devfileinfolist + [
    {'src': 'lib/dynamic/liborx*.dylib','dst': 'lib'}
]

linuxfileinfolist = devfileinfolist + [
    {'src': 'lib/dynamic/liborx*.so',   'dst': 'lib'}
]

# List of folder info
docfolderinfolist = [
    {'src': '../doc/html',              'dst': None}
]

devfolderinfolist = [
    {'src': 'include',                  'dst': None}
]

codefolderinfolist = [
    {'src': 'bin',                      'dst': None},
    {'src': 'build/android',            'dst': None},
    {'src': 'build/android-native',     'dst': None},
    {'src': 'build/ios',                'dst': None},
    {'src': 'build/linux',              'dst': None},
    {'src': 'build/mac',                'dst': None},
    {'src': 'build/windows',            'dst': None},
    {'src': 'demo',                     'dst': None},
    {'src': 'include',                  'dst': None},
    {'src': 'lib',                      'dst': None},
    {'src': 'plugins',                  'dst': None},
    {'src': 'src',                      'dst': None},
]


### Platform info

platforminfolist = {

    'vs2008': {
        'name':             'dev-vs2008',
        'filename':         'dev-vs2008',
        'format':           'zip',
        'fileinfolist':     vsfileinfolist,
        'folderinfolist':   devfolderinfolist
    },

    'vs2010': {
        'name':             'dev-vs2010',
        'filename':         'dev-vs2010',
        'format':           'zip',
        'fileinfolist':     vsfileinfolist,
        'folderinfolist':   devfolderinfolist
    },

    'mingw': {
        'name':             'dev-mingw',
        'filename':         'dev-mingw',
        'format':           'zip',
        'fileinfolist':     mingwfileinfolist,
        'folderinfolist':   devfolderinfolist
    },

    'mac': {
        'name':             'dev-mac',
        'filename':         'dev-mac',
        'format':           'zip',
        'fileinfolist':     macfileinfolist,
        'folderinfolist':   devfolderinfolist
    },

    'linux32': {
        'name':             'dev-linux32',
        'filename':         'dev-linux32',
        'format':           'bztar',
        'fileinfolist':     linuxfileinfolist,
        'folderinfolist':   devfolderinfolist
    },

    'linux64': {
        'name':             'dev-linux64',
        'filename':         'dev-linux64',
        'format':           'bztar',
        'fileinfolist':     linuxfileinfolist,
        'folderinfolist':   devfolderinfolist
    },

    'doxygen': {
        'name':             'doc',
        'filename':         'doc',
        'format':           'zip',
        'fileinfolist':     basefileinfolist,
        'folderinfolist':   docfolderinfolist
    },

    'src': {
        'name':             'code',
        'filename':         'src',
        'format':           'zip',
        'fileinfolist':     codefileinfolist,
        'folderinfolist':   codefolderinfolist
    }
}


### Prepares

# Deletes packing folder if present
if os.path.exists(workdir):
    shutil.rmtree(workdir)

# Gets base destination path
basedst = os.path.join(workdir, os.path.join('orx-' + version, platforminfolist[platform]['name']))


### Copies folders

# Should copy folders?
if platforminfolist[platform]['folderinfolist']:

    # Creates destination path if needed
    if not os.path.exists(basedst):
        os.makedirs(basedst)

    # For all folder info for this platform
    for folderinfo in platforminfolist[platform]['folderinfolist']:

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
        shutil.copytree(os.path.join(basesrc, folderinfo['src']), destination)


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
