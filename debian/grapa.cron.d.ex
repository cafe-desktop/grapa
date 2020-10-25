#
# Regular cron jobs for the grapa package
#
0 4	* * *	root	[ -x /usr/bin/grapa_maintenance ] && /usr/bin/grapa_maintenance
