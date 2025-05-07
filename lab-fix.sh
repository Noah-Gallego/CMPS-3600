
DIRECTORY=3600
echo "Setting the permissions of your ~/$DIRECTORY directory..."
# lock out world at the root folder level, only allow faculty and owner.
chmod  770 ~/$DIRECTORY
# set any files that are executable by 'user' to be executable by 'everyone'
find ~/$DIRECTORY -type f -perm -u=x -exec chmod go+x '{}' \; 2> /dev/null
#cp ~/.bash_history ~/assignments
#allow faculty read and write access
chmod -R ugo+rw,a+r ~/$DIRECTORY/1/ 2>/dev/null
chmod -R ugo+rw,a+r ~/$DIRECTORY/2/ 2>/dev/null
chmod -R ugo+rw,a+r ~/$DIRECTORY/3/ 2>/dev/null
chmod -R ugo+rw,a+r ~/$DIRECTORY/4/ 2>/dev/null
chmod -R ugo+rw,a+r ~/$DIRECTORY/5/ 2>/dev/null
chmod -R ugo+rw,a+r ~/$DIRECTORY/6/ 2>/dev/null
chmod -R ugo+rw,a+r ~/$DIRECTORY/7/ 2>/dev/null
chmod -R ugo+rw,a+r ~/$DIRECTORY/8/ 2>/dev/null
chmod -R ugo+rw,a+r ~/$DIRECTORY/9/ 2>/dev/null
chmod -R ugo+rw,a+r ~/$DIRECTORY/a/ 2>/dev/null
chmod -R ugo+rw,a+r ~/$DIRECTORY/b/ 2>/dev/null
chmod -R ugo+rw,a+r ~/$DIRECTORY/c/ 2>/dev/null
chmod -R ugo+rw,a+r ~/$DIRECTORY/d/ 2>/dev/null
chmod -R ugo+rw,a+r ~/$DIRECTORY/e/ 2>/dev/null
chmod -R ugo+rw,a+r ~/$DIRECTORY/f/ 2>/dev/null
# make sure all directories are executable
find ~/$DIRECTORY/ -type d -exec chmod g+x '{}' \; 2> /dev/null
echo "complete."

