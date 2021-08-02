[compileoptions]
	-d__STDC_WANT_LIB_EXT1__=1	# Want clib extensions
	-wx			# Warnings at maximum level
	-we			# Warnings as errors
	-e25		# ?
	-zq			# Compile quietly, display only error messages
	-ze			# Enable extensions
	-od			# All optimisations disabled
	-d2			# Built with symbolic information
	-6r			# Pentium pro register calling conventions
	-bt=linux	# Build target is Linux
	-fo=.obj	# Object file extension is .obj
	-mf			# Flat memory model (small code/small data)

[sources]
	main.c
