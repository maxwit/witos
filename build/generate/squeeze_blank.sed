#!/bin/sed

/^$/{
	$!N
	/^\n$/D
}
