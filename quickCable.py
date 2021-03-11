#cableTool
import maya.cmds as cmds
import maya.mel as mel
import math

'''Store ui obj'''
divisionSlider = 'divisionSlider'
sideSlider = 'sideSlider'
radiusSlider = 'radiusSlider'
twistSlider = 'twistSlider'
strandXSlider = 'strandXSlider'
strandYSlider = 'strandYSlider'
chainNumSlider = 'chainNumSlider'
distSlider = 'distSlider'
chainCheckBox = 'chainCheckBox'
axisRadioButtonGrp = 'axisRadioButtonGrp'
cables = {}        ##list to store cables data

## Default parameters
default = {"curveName": "curve",
           "name": "cable",
           "chainMeshName": "mesh",
           "divisions": 20,
           "sides": 8,
           "radius": 0.5,
           "strandX": 1,
           "strandY": 1,
           "twist": 0,
           "chainNum" : 10,
           "dist": 1
           }

class Cable:
    def __init__(self, _curveName=default["curveName"],
                       _name=default["name"],
                       _chainMeshName=None,
                       _divisions=default["divisions"],
                       _sides=default["sides"],
                       _radius=default["radius"],
                       _strandX=default["strandX"],
                       _strandY=default["strandY"],
                       _chainNum=default["chainNum"],
                       _dist=default["dist"],
                       _twist=default["twist"]
                ):
        self.name = _name
        self.chainMeshName = _chainMeshName
        self.curveName = _curveName
        self.divisions = _divisions
        self.sides = _sides
        self.radius = _radius
        self.strandX = _strandX
        self.strandY = _strandY
        self.twist = _twist
        self.chainNum = _chainNum
        self.dist = _dist
        self.isChain = False

## read current slider input and make new data class object
def makeCableData(curveName,chainMeshName=None):
    newCableData = Cable()

    newCableData.curveName = str(curveName)       ## store curve name
    newCableName = "cable"+str(len(cables))
    newCableData.name = newCableName
    newCableData.chainMeshName = chainMeshName

    newCableData.divisions = cmds.intSliderGrp( divisionSlider, q=True, v=True )
    newCableData.sides = cmds.intSliderGrp( sideSlider, q=True, v=True )
    newCableData.radius = cmds.floatSliderGrp( radiusSlider, q=True, v=True )
    newCableData.strandX = cmds.intSliderGrp( strandXSlider, q=True, v=True )
    newCableData.strandY = cmds.intSliderGrp( strandYSlider, q=True, v=True )
    newCableData.twist = cmds.intSliderGrp( twistSlider, q=True, v=True )
    newCableData.chainNum = cmds.intSliderGrp( chainNumSlider, q=True, v=True )
    newCableData.dist = cmds.floatSliderGrp( distSlider, q=True, v=True )
    newCableData.isChain = cmds.checkBoxGrp( chainCheckBox, q=True, value1=True )

    cables[newCableName] = newCableData
    return newCableData

def makeCableDriver():
    ## check make chain or cable
    isChain = cmds.checkBoxGrp( chainCheckBox, q=True, value1=True )
    selected = cmds.ls(sl=True)

    if isChain==True:
        ## check if selected is a mesh
        if len(selected)==2:
            shapes1 = cmds.listRelatives(selected[0], shapes=True)  ##mesh
            shapes2 = cmds.listRelatives(selected[1], shapes=True)  ##curve
            if cmds.objectType(shapes1[0])=="mesh" and cmds.objectType(shapes2[0])=="nurbsCurve":
                curveName = selected[1]
                meshName = selected[0]
                ## if auto
                newCableData = makeCableData(curveName,meshName)
                makeChain(newCableData)
                return
        cmds.error("Please select a mesh and a curve.")
    else:
        ## check if selected is a curve
        if len(selected)==1:
            shapes = cmds.listRelatives(selected[0], shapes=True)
            if shapes:
                if cmds.objectType(shapes[0])=="nurbsCurve":
                    ## Store cable data
                    myCurve = selected[0]
                    newCableData = makeCableData(myCurve)
                    makeCable(newCableData)
                    return
        cmds.error("Please select a curve.")

def makeChain(cableData):
    ## store mesh
    tmpMeshName = cmds.duplicate(cableData.chainMeshName)
    mel.eval("CenterPivot;")
    mel.eval("move -rpr 0 0 0 ;")
    cmds.hide(cableData.chainMeshName)
    ## center it

    chainNum = computeChainNum(cableData)
    # print "chainNum=",chainNum
    # dist = getDefaultDist(cableData.chainMeshName)
    # print "dist=",dist
    createDiscGrid(tmpMeshName,1,chainNum,cableData.dist)
    ##rename it
    cmds.rename(cableData.name)
    cmds.select(cableData.name,replace=True)
    cmds.select(cableData.curveName,add=True)
    mel.eval("createCurveWarp;")
    cmds.select(cableData.name, replace=True)
    # mel.eval("FreezeTransformations;makeIdentity -apply true -t 1 -r 1 -s 1 -n 0 -pn 1;")
    mel.eval("CenterPivot;")

# Make cable based on data
def makeCable(cableData):

    ## create a disc
    cable = cmds.polyDisc(sides=cableData.sides, subdivisionMode=2, subdivisions=0, radius=cableData.radius)
    cmds.rename(cable,cableData.name)

    ## move it to the starting point of a curve
    infoNode = cmds.pointOnCurve(cableData.curveName, pr=0.0, ch=True, nn=True)
    infoNode1 = cmds.pointOnCurve(cableData.curveName, pr=0.1, ch=True, nn=True)
    position = cmds.getAttr(infoNode + ".position")[0]  # get the position of first point
    position1 = cmds.getAttr(infoNode1 + ".position")[0]
    normal = (position1[0]-position[0],position1[1]-position[1],position1[2]-position[2])

    # find angle between two vectors to rotate the disc
    angle = cmds.angleBetween( euler=True, v1=(0.0,1.0,0.0), v2=(normal[0],normal[1],normal[2]) )

    selected = cmds.ls(sl=True)
    disc = selected[0]

    cable = createDiscGrid(disc,cableData.strandX,cableData.strandY,cableData.radius)
    cmds.makeIdentity(apply=True, t=1, r=1, s=1, n=0)
    cmds.rename(cable,cableData.name)

    cmds.move(position[0], position[1], position[2], cableData.name)
    cmds.rotate(angle[0], angle[1], angle[2], cableData.name)

    cmds.select(cableData.name+".f[0:]")
    cmds.select(cableData.curveName, add=True)

    ## extrude
    cmds.polyExtrudeFacet(ch=True, inputCurve=cableData.curveName, d=cableData.divisions)
    cmds.select(cableData.name, replace=True)

    ## delete faces
    n = 2*cableData.strandX*cableData.strandY
    cmds.delete(cableData.name+".f[0:"+str(n-1)+"]")

    #cmds.select(cableData.name, replace=True)
    # update twist
    for node in cmds.listHistory(cableData.name):
        if cmds.nodeType(node)=="polyExtrudeFace":
            cmds.setAttr(str(node)+".twist", cableData.twist)

    cmds.polySoftEdge(angle=80, ch=True)
    cmds.select(cableData.name, replace=True)
    mel.eval("FreezeTransformations;makeIdentity -apply true -t 1 -r 1 -s 1 -n 0 -pn 1;")
    mel.eval("CenterPivot;")

#create a grid of disc at origin
def createDiscGrid(disc,x,y,radius):
    groupName = "tmpCableGRP"
    cmds.select(disc, r=True)
    if x==1 and y==1:
        return disc
    cmds.group(name=groupName)
    cmds.select(disc, r=True)
    if x>1:
        mel.eval("instance; move -r 0 0 {dist}; for ($i=1; $i<{n}; $i++) instance -st;".format(dist=2*radius,n=x-1))
    cmds.select(groupName, r=True)
    children = cmds.listRelatives(children=True)
    for child in children:
        cmds.select(child, r=True)
        if y>1:
            mel.eval("instance; move -r {dist} 0 0; for ($i=1; $i<{n}; $i++) instance -st;".format(dist=2*radius,n=y-1))

    mel.eval("polyUnite -ch 1 -mergeUVSets 1 -centerPivot -name {name}Combined {name};".format(name=groupName))
    mel.eval("DeleteHistory;")
    mel.eval("move -rpr 0 0 0 ;")   ## move to origin
    cmds.delete(disc)
    cmds.delete(groupName)

    return cmds.ls(sl=True)[0]

def getCurveLen(curveName):
    maxU = cmds.getAttr( curveName+".maxValue" )
    arcL= cmds.arcLengthDimension( curveName+'.u['+str(maxU)+']' )
    curveLen = cmds.getAttr( arcL + ".arcLength" )
    cmds.delete(arcL)
    return curveLen

def getDefaultDist(meshName):
    bbox = cmds.exactWorldBoundingBox(meshName)
    ## axis x y z
    axis = cmds.radioButtonGrp(axisRadioButtonGrp,q=True,select=True)
    if axis==1:
        objLen = bbox[3]-bbox[0]
    elif axis==2:
        objLen = bbox[4]-bbox[1]
    elif axis==3:
        objLen = bbox[5]-bbox[2]
    return objLen/2

def computeChainNum(cableData):
    curveLen = getCurveLen(cableData.curveName)
    return math.floor( curveLen / (cableData.dist*2) )

# Load selected cable attribute to UI
def load():
    sel = cmds.ls(sl=True)
    if len(sel)!=1 or cables.get(sel[0])==None:
        cmds.error("Please select one cable to load.")
        return
    currData = cables[str(sel[0])]
    for node in cmds.listHistory(sel[0]):
        if cmds.nodeType(node)=="polyExtrudeFace":
            #divisions = cmds.getAttr(node+".divisions")
            cmds.intSliderGrp( divisionSlider, e=True, v=cmds.getAttr(node+".divisions") )
            cmds.intSliderGrp( twistSlider, e=True, v=cmds.getAttr(node+".twist") )
        if cmds.nodeType(node)=="polyDisc":
            cmds.intSliderGrp( sideSlider, e=True, v=cmds.getAttr(node+".sides") )
            cmds.floatSliderGrp( radiusSlider, e=True, v=cmds.getAttr(node+".radius") )
    cmds.intSliderGrp( strandXSlider, e=True, v=currData.strandX )
    cmds.intSliderGrp( strandYSlider, e=True, v=currData.strandY )
    cable = cables[str(sel[0])]

def edit():
    mel.eval("CurveEditTool")

def getSelCableData():
    sel = cmds.ls(sl=True)
    if len(sel)!=1 or cables.get(sel[0])==None:
        return None
    return cables[str(sel[0])]

def rebuildCable():
    currData = getSelCableData()        ##find cable data
    if currData!=None:
        cables.pop(currData.name)       ##delete from dictionary
        cmds.delete()                   ##delete original cable
        if(currData.isChain==True):
            cmds.select(currData.chainMeshName, replace=True)       ##select the curve
            cmds.select(currData.curveName, add=True)               ##select the curve
        else:
            cmds.select(currData.curveName, replace=True)           ##select the curve

        makeCableDriver()
    else:
        print "no cable data found"

def resetUI():
    print "RESETED"
    cmds.intSliderGrp( divisionSlider, e=True, v=default["divisions"] )
    cmds.intSliderGrp( sideSlider, e=True, v=default["sides"] )
    cmds.floatSliderGrp( radiusSlider, e=True, v=default["radius"] )
    cmds.intSliderGrp( strandXSlider, e=True, v=default["strandX"] )
    cmds.intSliderGrp( strandYSlider, e=True, v=default["strandY"] )
    cmds.intSliderGrp( twistSlider, e=True, v=default["twist"] )

def strandOn(*arg):
    cmds.intSliderGrp(strandXSlider, e=True, enable=True)
    cmds.intSliderGrp(strandYSlider, e=True, enable=True)

def strandOff(*arg):
    cmds.intSliderGrp(strandXSlider, e=True, enable=False)
    cmds.intSliderGrp(strandYSlider, e=True, enable=False)

def chainOn(*arg):
    cmds.intSliderGrp(chainNumSlider, e=True, enable=True)
    cmds.floatSliderGrp(distSlider, e=True, enable=True)
    cmds.intSliderGrp(divisionSlider, e=True, enable=False)
    cmds.intSliderGrp(sideSlider, e=True, enable=False)
    cmds.floatSliderGrp(radiusSlider, e=True, enable=False)

def chainOff(*arg):
    cmds.intSliderGrp(chainNumSlider, e=True, enable=False)
    cmds.floatSliderGrp(distSlider, e=True, enable=False)
    cmds.intSliderGrp(divisionSlider, e=True, enable=True)
    cmds.intSliderGrp(sideSlider, e=True, enable=True)
    cmds.floatSliderGrp(radiusSlider, e=True, enable=True)

def commonCableUI(columnWidth):
    cmds.columnLayout(adjustableColumn=True)
    cmds.intSliderGrp(divisionSlider, field=True, label='Divisions:  ', minValue=1, maxValue=200,
                                    value=20, changeCommand="rebuildCable()", columnWidth3=columnWidth)
    cmds.intSliderGrp(sideSlider, field=True, label='Sides:  ', minValue=3, maxValue=50,
                                    value=8, changeCommand="rebuildCable()", columnWidth3=columnWidth )
    cmds.floatSliderGrp(radiusSlider, field=True, label='Radius:  ', minValue=0, maxValue=5, step=0.01,
                                    value=0.5, changeCommand="rebuildCable()", columnWidth3=columnWidth )
    cmds.intSliderGrp(twistSlider, field=True, label='Twist:  ', minValue=0, maxValue=3000,
                                    value=0, changeCommand="rebuildCable()", columnWidth3=columnWidth )
    cmds.setParent("..")

def strandCableUI(columnWidth):
    cmds.columnLayout(adjustableColumn=True)
    cmds.checkBoxGrp( numberOfCheckBoxes=1, label='Strand:  ',
                      onCommand1="strandOn()",offCommand1="strandOff()" )
    cmds.intSliderGrp(strandXSlider, field=True, label='Strand X:  ', minValue=1, maxValue=15,
                                    value=1, changeCommand="rebuildCable()", columnWidth3=columnWidth,
                                    enable=False )
    cmds.intSliderGrp(strandYSlider, field=True, label='Strand Y:  ', minValue=1, maxValue=15,
                                    value=1, changeCommand="rebuildCable()", columnWidth3=columnWidth,
                                    enable=False )
    cmds.setParent("..")

def chainCableUI(columnWidth):
    cmds.columnLayout(adjustableColumn=True)
    cmds.checkBoxGrp( chainCheckBox, numberOfCheckBoxes=1, label='Chains:  ',
                      onCommand1="chainOn()",offCommand1="chainOff()" )
    # cmds.checkBoxGrp(numberOfCheckBoxes=1, label='Auto Fill:  ' )
    # cmds.radioButtonGrp(axisRadioButtonGrp, label='Axis:  ', labelArray3=['X', 'Y', 'Z'], numberOfRadioButtons=3,select=1 )
    cmds.intSliderGrp(chainNumSlider, field=True, label='Chain Number:  ', minValue=1, maxValue=200,
                                    value=default["chainNum"], changeCommand="rebuildCable()", columnWidth3=columnWidth,
                                    enable=False )
    cmds.floatSliderGrp(distSlider, field=True, label='Distance:  ', minValue=0, maxValue=100,
                                    value=default["dist"], changeCommand="rebuildCable()", columnWidth3=columnWidth,
                                    enable=False )
    cmds.setParent("..")

def quickCableUI():
    winName = "cableUI"
    width = 400
    height = 500

    if(cmds.window(winName, q=True, ex=True)):
        cmds.deleteUI(winName)
    window = cmds.window(winName, title="Quick Cable", widthHeight=(width,height), menuBar=True, s=True)
    cmds.menu( label='Options', tearOff=True )
    cmds.menuItem( label='Reset', command="resetUI()" )

    form = cmds.formLayout('windowsform',p=winName)
    layout = cmds.columnLayout(adjustableColumn=True)
    cmds.formLayout(form,e=True,af=[(layout,'top',10),(layout,'left',10),(layout,'right',10)])

    columnWidth=[90,50,100]
    cmds.frameLayout('Common',collapsable=True,bgc=[0.3,0.3,0.3])
    commonCableUI(columnWidth)
    cmds.setParent("..")

    cmds.frameLayout('Strand',collapsable=True)
    strandCableUI(columnWidth)
    cmds.setParent("..")

    cmds.frameLayout('Chain',collapsable=True)
    chainCableUI(columnWidth)
    cmds.setParent("..")

    cmds.separator( h=10,style='double' )

    row = cmds.rowLayout(numberOfColumns=4)
    cmds.button(label="Cable it", command="makeCableDriver()", width = 90)
    cmds.button(label="Rebuild", command="rebuildCable()", width = 90)
    cmds.button(label="Load Cable", command="load()")
    cmds.button(label="Edit Curve", command="edit()")

    cmds.setParent("..")
    cmds.showWindow(window)


quickCableUI()
