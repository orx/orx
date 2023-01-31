.\gradlew build publishToMavenLocal
xcopy /S /I %USERPROFILE%\.m2\repository\org\orx-project %ORX%\lib\static\android\org\orx-project\
pause
