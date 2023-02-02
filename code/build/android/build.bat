gradlew build publishToMavenLocal

xcopy /S /I %USERPROFILE%\.m2\repository\org\orx-project ..\..\lib\static\android\repository\org\orx-project

pause
