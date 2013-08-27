#################################################################################
# ngBot - Auto Announce Top Uploaders                                           #
#################################################################################
# 
# Description:
# - Auto announces the top uploaders at a configurable interval.
#
# Installation:
# 1. Add the following to your eggdrop.conf:
#    source pzs-ng/plugins/Top.tcl
#
# 2. Rehash or restart your eggdrop for the changes to take effect.
#
# Changelog:
# - 20110913 - Sked:	Fixed output for users with chars other than A-Za-z0-9_ - Fix by PCFiL
# - 20120529 - Madjik:  Modded the output for two lines with header
#                       Added support for displaying users group
#
#################################################################################

namespace eval ::ngBot::plugin::Top {
	variable ns [namespace current]
	variable np [namespace qualifiers [namespace parent]]

	variable top

	## Config Settings ###############################
	##
	## Interval between announces in seconds (default: 7200 - 2 hours)
	set top(interval)   7200
	##
	## Section to display (0 = DEFAULT)
	set top(sect)       0
	##
	## Maximum number of users to display
	set top(users)      5
	##
	## Message prefix
	set top(prefix)     "\002charts\002"
	set top(header)     "$top(prefix) > This week awesome people >"

	##
	## Output channels
	set top(chan)       "#YOURCHAN"
	##
	##################################################

	set top(version) "20120529"

	variable timer


	proc init {args} {
		variable top
		[namespace current]::startTimer
		putlog "\[ngBot\] Top :: Loaded successfully (Version: $top(version))."
	}

	proc deinit {args} {
		[namespace current]::killTimer

		namespace delete [namespace current]
	}

	proc killTimer {} {
		variable timer

		if {[catch {killutimer $timer} error] != 0} {
			putlog "\[ngBot\] Top :: Warning: Unable to kill announce timer \"$error\""
		}
	}

	proc startTimer {} {
		variable top

		variable timer [utimer $top(interval) "[namespace current]::showTop"]
	}

	proc showTop {args} {
		variable np
		variable ns
		variable top
		variable ${np}::binary
		variable ${np}::location

		[namespace current]::startTimer

		if {[catch {exec $binary(STATS) -r $location(GLCONF) -u -w -x $top(users) -s $top(sect)} output] != 0} {
			putlog "\[ngBot\] Top :: Error: Problem executing stats-exec \"$output\""
			return
		}

		set msg [list]
		foreach line [split $output "\n"] {
			regsub -all -- {(\s+)\s} $line " " line

			if {[regexp -- {^\[(\d+)\] (.*?) (.*?) (\d+) (\d+)\w+ (\S+)} $line -> pos username tagline files bytes speed]} {
				set groups [usergrp $username]
				lappend msg "$pos. $username/$groups \002$bytes\002MB "
			}
		}

		if {[llength $msg] == 0} {
			set msg "Not enough data in the pipe yet..."
		}

		foreach chan [split $top(chan)] {
			putquick "PRIVMSG $chan : "
			putquick "PRIVMSG $chan :$top(header)"
			putquick "PRIVMSG $chan :$top(prefix) > [join $msg " ! "]"
			putquick "PRIVMSG $chan : "
		}
	}

	proc usergrp {user} {
		variable np
	        variable ${np}::location

		set groups [list]
		if {![catch {set handle [open "$location(USERS)/$user" r]} error]} {
			set data [read $handle]
			close $handle
			foreach line [split $data "\n"] {
				switch -exact -- [lindex $line 0] {
					"GROUP" {lappend groups [lindex $line 1]}
				}
			}
			return $groups
		} else {
			putlog "\[Top\] Error :: Unable to open user file for \"$user\" ($error)."
			return false
		}
	}

}
