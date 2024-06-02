import bpy
from math import pi
import json
import csv

directionKeys = ["x+", "x-", "y+", "y-", "z+", "z-"]
tileSize = 2
offsetsX = [tileSize, -tileSize, 0, 0, 0, 0]
offsetsY = [0, 0, -tileSize, tileSize, 0, 0]
offsetsZ = [0, 0, 0, 0, tileSize, -tileSize]
    
def getLocString(location):
    return str(round(location[0])) + "," + str(round(location[1])) + "," + str(round(location[2]))

def getRot(obj):
    return (round(obj.rotation_euler[2] / pi * 180 / 90) + 4) % 4

def getNoNumberName(obj):
    dotIndex = obj.name.index(".")
    noNumberName = obj.name[0:dotIndex]
    return noNumberName

    
def to_json(obj):
    return json.dumps(obj, default=lambda obj: obj.__dict__)
    
class TileStats:
    Name = ""
    tileName = ""
    rotation = 0
    mappings = {}
    
    def __init__(self, name, rot, mappings):
        self.tileName = name + "_" + str(rot)
        self.Name = self.tileName
        self.rotation = rot
        self.mappings = mappings
        
    def __str__(self):
        return "Tile: " + self.tileName + ", rot: " + str(self.rotation) + ", mappings: " + str(self.mappings)
    
    
    
    
class Tile:
    location = [0, 0, 0]
    rotation = 0
    name = ""
    nameWithRot = ""
    
    def __init__(self, name, loc, rot):
        self.location = loc
        self.name = name
        self.rotation = rot
        self.nameWithRot = self.name + "_" + str(self.rotation)
        
    def __str__(self):
        return "Tile: {:20s}, loc: {:20s}, rot: {:2d}".format(self.name, str(self.location), self.rotation)
            
    
class WFCAnalyzeOperator(bpy.types.Operator):
    
    bl_idname = "object.wfc_analyze"
    bl_label = "Analyze WFC selection"

    @classmethod
    def poll(cls, context):
        return context.active_object is not None


    def execute(self, context):
        
        tilesDict = {}
        gridDict = {}
        
        
        for obj in bpy.context.selected_objects:
            noNumberName = getNoNumberName(obj)
            print(noNumberName)
            rot_z = getRot(obj)
            print(rot_z)
            withRotName = noNumberName + "_" + str(rot_z)
            locString = getLocString(obj.location)
            gridDict[locString] = Tile(noNumberName, [obj.location[0], obj.location[1], obj.location[2]], rot_z)
            
            if withRotName not in tilesDict.keys():
                tilesDict[withRotName] = TileStats(noNumberName, rot_z, {"x+": {"dirToCountMap":{}}, "x-": {"dirToCountMap":{}}, "y+": {"dirToCountMap":{}}, "y-": {"dirToCountMap":{}}, "z+": {"dirToCountMap":{}}, "z-": {"dirToCountMap":{}}})
            
        #tilesDict["Empty_0"] = TileStats("Empty", 0, {"x+": {"dirToCountMap":{}}, "x-": {"dirToCountMap":{}}, "y+": {"dirToCountMap":{}}, "y-": {"dirToCountMap":{}}, "z+": {"dirToCountMap":{}}, "z-": {"dirToCountMap":{}}})
            
        testTile = TileStats("test tile", 2, {
            "up": ["one", "two"]
        })
        
        for k in tilesDict.keys():
            print(k + ": " + str(tilesDict[k]))
            
        print(gridDict.keys())
            
        for obj in bpy.context.selected_objects:
            noNumberName = getNoNumberName(obj)
            print(noNumberName)
            rot_z = getRot(obj)
            print(rot_z)
            withRotName = noNumberName + "_" + str(rot_z)
            locString = getLocString(obj.location)
            thisTileRef = gridDict[locString]
            
            for i in range(0,6):
                generatedLoc = [obj.location[0] + offsetsX[i], obj.location[1] + offsetsY[i], obj.location[2] + offsetsZ[i]]
                locStr = getLocString(generatedLoc)
                if locStr not in gridDict.keys():
                    continue
                    
                neighbourTileRef = gridDict[locStr]
                
                #if neighbourTileRef.nameWithRot == "NotEmpty_0":
                #    #print("It is not empty")
                #    #print(withRotName)
                #    oppositeIndex = i+1 if i%2 == 0 else i-1
                #    tilesDict[withRotName].mappings[directionKeys[i]]["dirToCountMap"]["Empty_0"] = 1
                #    tilesDict["Empty_0"].mappings[directionKeys[oppositeIndex]]["dirToCountMap"][withRotName] = 1
                #    continue
                
                if neighbourTileRef.nameWithRot not in tilesDict[withRotName].mappings[directionKeys[i]]["dirToCountMap"].keys():
                    tilesDict[withRotName].mappings[directionKeys[i]]["dirToCountMap"][neighbourTileRef.nameWithRot] = 1
                else:
                    tilesDict[withRotName].mappings[directionKeys[i]]["dirToCountMap"][neighbourTileRef.nameWithRot] += 1
                
        
        #for i in range(0, 6):
        #    tilesDict["Empty_0"].mappings[directionKeys[i]]["dirToCountMap"]["Empty_0"] = 1
    
        
        for k1,v1 in tilesDict.items():
            for k2, v2 in v1.mappings.items():
                sum = 0
                for k3, v3 in v2["dirToCountMap"].items():
                    sum += v3
                for k3, v3 in v2["dirToCountMap"].items():
                    v2["dirToCountMap"][k3] /= sum
                        
        
        #if "NotEmpty_0" in tilesDict.keys():
        #    del tilesDict["NotEmpty_0"]
        
        jsonDict = {}
        for k,v in tilesDict.items():
            jsonDict[k] = to_json(v)
            
            
        #for k in tilesDict.keys():
        #    print(k + ": " + str(tilesDict[k]))
        
        json_object = json.dumps(list(jsonDict.values()), indent=2)
 
        filepath = "C:\\Users\\Alen Leban\\Documents\\Unreal Projects\\InfiniteFRI\\tileStats.json"
 
        # Writing to sample.json
        with open(filepath, "w") as outfile:
            outfile.write(json_object)
        
        with open(filepath) as f:
            newText=f.read().replace('\\"', '\"').replace('\"{', '{').replace('}\"', '}')


        with open(filepath, "w") as f:
            f.write(newText)

            
        #with open(filepath, 'w', newline='') as csvfile:
        #    spamwriter = csv.writer(csvfile, delimiter=',',
        #                            quotechar='"', quoting=csv.QUOTE_MINIMAL)
        #    spamwriter.writerow(["---", "tileName", "rotation", "mappings"])
        #    for k,v in tilesDict.items():
        #        withRotName = v.tileName + "_" + str(v.rotation)
        #        spamwriter.writerow([withRotName, withRotName, v.rotation, v.mappings])
        
        
        
        
        self.report({'INFO'}, 'Printing report to Info window.')
        
        
        
        return {'FINISHED'}

    
    
    

class TestPanel(bpy.types.Panel):
    bl_label = "WFCAnalyzer"
    bl_idname = "PT_TestPanel"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "WFCAnalyzer"
    
    def draw(self, context):
        layout = self.layout
        
        row = layout.row()
        row.label(text = "Actions")
        row = layout.row()
        row.operator("object.wfc_analyze")
        
        
def register():
    bpy.utils.register_class(WFCAnalyzeOperator)
    bpy.utils.register_class(TestPanel)
    
def unregister():
    bpy.utils.unregister_class(WFCAnalyzeOperator)
    bpy.utils.unregister_class(TestPanel)
    
if __name__ == "__main__":
    register()
    
    
    
