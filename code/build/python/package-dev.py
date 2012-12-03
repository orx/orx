### Imports

import glob, shutil, os, sys, argparse


### Constants

# List of available platforms
platformlist    = ['vs2008', 'vs2010', 'mingw', 'mac', 'linux32', 'linux64', 'doxygen']

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
docfileinfolist = [
    {'src': '../AUTHORS',               'dst': None},
    {'src': '../CHANGELOG',             'dst': None},
    {'src': '../COPYRIGHT',             'dst': None},
    {'src': '../LICENSE',               'dst': None},
    {'src': '../README',                'dst': None}
]

devfileinfolist = docfileinfolist + [
    {'src': 'bin/*Template.ini',        'dst': None},
    {'src': 'bin/orx*.ini',             'dst': None}
]

vsfileinfolist = devfileinfolist + [
    {'src': 'bin/orx*.exe',             'dst': None},
    {'src': 'bin/orx*.dll',             'dst': None},
    {'src': 'lib/dynamic/orx*.lib',     'dst': 'lib'}
]

mingwfileinfolist = devfileinfolist + [
    {'src': 'bin/orx*.exe',             'dst': None},
    {'src': 'bin/orx*.dll',             'dst': None},
    {'src': 'lib/dynamic/liborx*.a',    'dst': 'lib'}
]

macfileinfolist = devfileinfolist + [
    {'src': 'bin/orx',                  'dst': None},
    {'src': 'bin/orxp',                 'dst': None},
    {'src': 'bin/orxd',                 'dst': None},
    {'src': 'bin/liborx*.dylib',        'dst': None},
    {'src': 'lib/dynamic/liborx*.dylib','dst': 'lib'}
]

linuxfileinfolist = devfileinfolist + [
    {'src': 'bin/orx',                  'dst': None},
    {'src': 'bin/orxp',                 'dst': None},
    {'src': 'bin/orxd',                 'dst': None},
    {'src': 'bin/liborx*.so',           'dst': None},
    {'src': 'lib/dynamic/liborx*.so',   'dst': 'lib'}
]

# List of folders
docfolderlist = [
    '../doc/html'
]

devfolderlist = [
    'include'
]


### Platform info

platforminfolist = {

    'vs2008': {
        'name':         'dev-vs2008',
        'format':       'zip',
        'fileinfolist': vsfileinfolist,
        'folderlist':   devfolderlist
    },

    'vs2010': {
        'name':         'dev-vs2010',
        'format':       'zip',
        'fileinfolist': vsfileinfolist,
        'folderlist':   devfolderlist
    },

    'mingw': {
        'name':         'dev-mingw',
        'format':       'zip',
        'fileinfolist': mingwfileinfolist,
        'folderlist':   devfolderlist
    },

    'mac': {
        'name':         'dev-mac',
        'format':       'zip',
        'fileinfolist': macfileinfolist,
        'folderlist':   devfolderlist
    },

    'linux32': {
        'name':         'dev-linux32',
        'format':       'bztar',
        'fileinfolist': linuxfileinfolist,
        'folderlist':   devfolderlist
    },

    'linux64': {
        'name':         'dev-linux64',
        'format':       'bztar',
        'fileinfolist': linuxfileinfolist,
        'folderlist':   devfolderlist
    },

    'doxygen': {
        'name':         'doc',
        'format':       'zip',
        'fileinfolist': docfileinfolist,
        'folderlist':   docfolderlist
    }
}


### Cleans

# Deletes packing folder if present
if os.path.exists(workdir):
    shutil.rmtree(workdir)


### Copies files

# Gets base destination path
basedst = os.path.join(workdir, os.path.join('orx-' + version, platforminfolist[platform]['name']))

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


### Copies folders

# For all folder info for this platform
for folder in platforminfolist[platform]['folderlist']:

    # Does the destination folder exist?
    if os.path.exists(os.path.join(basedst, folder)):

        # Deletes it
        shutil.rmtree(os.path.join(basedst, folder))

    # Logs
    print('Copying folder: ' + folder)

    # Copies folder
    shutil.copytree(os.path.join(basesrc, folder), os.path.join(basedst, folder))


### Exports archive

# Creates archive from folder
archive = shutil.make_archive(os.path.join('packages', 'orx-' + platforminfolist[platform]['name'] + '-' + version), platforminfolist[platform]['format'], workdir, '.')

# Logs
print('Created archive: ' + os.path.basename(archive))
