[target_name]
	srtx

[target_type]
	executable

[compile_options]
	-wx			# Warnings at maximum level
	-we			# Warnings as errors
	-e25		# Max 25 errors before stopping compilation
	-zq			# Compile quietly, display only error messages
	-ze			# Enable extensions
	-od			# All optimisations disabled
	-d2			# Built with symbolic information
	-fo=.obj	# Object file extension is .obj

[sources]
	main.c
