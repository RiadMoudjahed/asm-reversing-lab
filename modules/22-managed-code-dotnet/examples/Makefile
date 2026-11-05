# Builds the module 22 examples (needs the .NET SDK and ilspycmd of the lab image).
BUILD := build

all: $(BUILD)/Parcels.dll $(BUILD)/debug/Parcels.dll

$(BUILD)/Parcels.dll: 22_parcels/Program.cs 22_parcels/22_parcels.csproj
	mkdir -p $(BUILD)
	dotnet build 22_parcels/22_parcels.csproj -c Release -o $(BUILD) -nologo -v quiet

# The same program with a portable PDB, which keeps the names of local variables.
$(BUILD)/debug/Parcels.dll: 22_parcels/Program.cs 22_parcels/22_parcels.csproj
	mkdir -p $(BUILD)/debug
	dotnet build 22_parcels/22_parcels.csproj -c Release -o $(BUILD)/debug -p:DebugType=portable -nologo -v quiet

run: all
	dotnet $(BUILD)/Parcels.dll

.PHONY: all run
