#
# Author of script: Gordon Griesel
# Script code based on similar scripts written by Michael Sarr.
#
# This script will set permissions on student directories.
# Permissions are set to files in the student's course folder.
# Write permissions are granted to the "faculty" group.
#
# This script is run by the student when lab starts.
# Path is currently:
#     /home/fac/gordon/p/3600/lab-start.sh 
#
#
# During a lab or at end of lab, run:
#     /home/fac/gordon/p/3600/lab-fix.sh 
#   

DIRECTORY=3600

echo
echo
echo "Setting the permissions of your ~/$DIRECTORY directory..."

# lock out world at the root folder level, only allow owner and faculty.
chmod  770 ~/$DIRECTORY

# set the group id on main directory
chmod  g+s ~/$DIRECTORY

# make sure all sub-directories are executable
find ~/$DIRECTORY/ -type d -exec chmod g+x '{}' \; 2> /dev/null

# make sure all sub-directories have gid set
find ~/$DIRECTORY/ -type d -exec chmod g+s '{}' \; 2> /dev/null

# set any files that are executable by 'user' to be executable by 'everyone'
# I don't need this. -Gordon
#find ~/$DIRECTORY -type f -perm -u=x -exec chmod go+x '{}' \; 2> /dev/null

# This would be useful to see, after a lab is over sometimes.
# This may be implemented as needed.
#cp ~/.bash_history ~/$DIRECTORY

#allow Gordon at least read access - all files
chmod -R ug+r ~/$DIRECTORY/1/ 2> /dev/null
chmod -R ug+r ~/$DIRECTORY/2/ 2> /dev/null
chmod -R ug+r ~/$DIRECTORY/3/ 2> /dev/null
chmod -R ug+r ~/$DIRECTORY/4/ 2> /dev/null
chmod -R ug+r ~/$DIRECTORY/5/ 2> /dev/null
chmod -R ug+r ~/$DIRECTORY/6/ 2> /dev/null
chmod -R ug+r ~/$DIRECTORY/7/ 2> /dev/null
chmod -R ug+r ~/$DIRECTORY/8/ 2> /dev/null
chmod -R ug+r ~/$DIRECTORY/9/ 2> /dev/null
chmod -R ug+r ~/$DIRECTORY/a/ 2> /dev/null
chmod -R ug+r ~/$DIRECTORY/b/ 2> /dev/null
chmod -R ug+r ~/$DIRECTORY/c/ 2> /dev/null
chmod -R ug+r ~/$DIRECTORY/d/ 2> /dev/null
chmod -R ug+r ~/$DIRECTORY/e/ 2> /dev/null
chmod -R ug+r ~/$DIRECTORY/f/ 2> /dev/null
chmod -R ug+r ~/$DIRECTORY/proj/ 2> /dev/null

echo "."
echo "complete."
echo 


