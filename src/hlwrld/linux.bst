[target_name]
	hlwrld

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
	-bt=linux	# Build target is Linux
	-fo=.obj	# Object file extension is .obj

[compile_options:platform=LINUX]
	-6r			# Pentium pro register calling conventions
	-mf			# 32-bit flat memory model (small code/small data)

[sources]
	main.c
