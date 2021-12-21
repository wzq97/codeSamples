using UnityEngine;
using UnityEditor;
using System.Collections;
using System.Collections.Generic;

[CustomEditor(typeof(PrefabPainter))]
public class PrefabPainterEditor : EditorWindow
{
    PrefabPainter _pp;

    Vector3 mousePos;
    Vector3 hitPoint;
    Vector3 hitNormal;

    Rect dropArea;
    int _previewSize = 75;
    int _previewPadding = 3;

    int toolbarInt = 1;
    string[] toolbarStrings = { "Select", "Paint" };

    [MenuItem("Tools/PrefabPainter")]
    static void Init()
    {
        PrefabPainterEditor window = (PrefabPainterEditor)GetWindow(typeof(PrefabPainterEditor),
            false, "Prefab Painter");
        window.minSize = new Vector2(400, 450);
        window.Show();
    }

    private void OnEnable()
    {
        SceneView.duringSceneGui += OnScene;     // Subscribe OnSceneGUI
        if (_pp == null)
        {
            _pp = CreateInstance<PrefabPainter>();
        }
        _pp.LoadAllAssets();
    }

    private void OnDisable()
    {
        SceneView.duringSceneGui -= OnScene;
    }

    // Handle events when mouse is in the scene window
    private void OnScene(SceneView scene)
    {
        //Debug.Log("OnSceneGUI");
        if (_pp.mode != PrefabPainter.MODE.PAINT)
            return;
        //Selection.activeGameObject = null;                      //Deselect everything
        HandleUtility.AddDefaultControl(GUIUtility.GetControlID(FocusType.Passive));
        Event e = Event.current;

        mousePos = e.mousePosition;
        mousePos.y = scene.camera.pixelHeight - mousePos.y;     // Setup scene camera
        Ray ray = scene.camera.ScreenPointToRay(mousePos);
        RaycastHit hit;
        if (Physics.Raycast(ray, out hit))
        {
            Handles.color = Color.red;
            Handles.DrawWireDisc(hit.point, hit.normal, _pp.radius);
            hitPoint = hit.point;
            hitNormal = hit.normal;
            scene.Repaint();        // Refresh the scene
        }
        else
        {
            return;
        }

        if (!e.alt && (e.type == EventType.MouseDown || e.type == EventType.MouseDrag) && e.button == 0)
        {
            _pp.Paint(hitPoint, hitNormal);
        }
    }

    // GUI Methods
    private void OnGUI()
    {
        GUILayout.Label("Painting Mode", EditorStyles.boldLabel);
        toolbarInt = GUILayout.Toolbar(toolbarInt, toolbarStrings);
        _pp.mode = (PrefabPainter.MODE)toolbarInt;
        //_pp.mode = (PrefabPainter.MODE)EditorGUILayout.EnumPopup(_pp.mode);
        _pp.singlePlacement = EditorGUILayout.Toggle("Single Placement", _pp.singlePlacement);
        GUILayout.Space(15);

        GUILayout.Label("Brush Settings", EditorStyles.boldLabel);
        _pp.radius = EditorGUILayout.Slider("Radius", _pp.radius, 0.01f, 30f);
        _pp.density = EditorGUILayout.IntField("Density", _pp.density);
        GUILayout.Space(10);

        GUILayout.Label("Prefabs", EditorStyles.boldLabel);
        DropAreaGUI();
        if (GUILayout.Button("Remove", GUILayout.Width(65)))
            _pp.RemoveSelected();
        GUILayout.Space(10);

        string currObjName = "";
        if (_pp.paintAssetsDatabase.Count > 0)
            currObjName = _pp.GetAsset(_pp.currSelIndex).asset.name;
        GUILayout.Label(currObjName, EditorStyles.boldLabel);
        GUILayout.Space(10);

        GUILayout.Label("Prefab Settings", EditorStyles.boldLabel);
        if(_pp.paintAssetsDatabase.Count > 0)
        {
            _pp.paintAssetsDatabase[_pp.currSelIndex].scaleMin = EditorGUILayout.Slider("Scale Min", _pp.paintAssetsDatabase[_pp.currSelIndex].scaleMin, 0.01f, 5f);
            _pp.paintAssetsDatabase[_pp.currSelIndex].scaleMax = EditorGUILayout.Slider("Scale Max", _pp.paintAssetsDatabase[_pp.currSelIndex].scaleMax, 0.01f, 5f);
            _pp.paintAssetsDatabase[_pp.currSelIndex].randomYaw = EditorGUILayout.Toggle("Random Yaw", _pp.paintAssetsDatabase[_pp.currSelIndex].randomYaw);
            _pp.paintAssetsDatabase[_pp.currSelIndex].alignToNormal = EditorGUILayout.Toggle("Align to Normal", _pp.paintAssetsDatabase[_pp.currSelIndex].alignToNormal);
            _pp.paintAssetsDatabase[_pp.currSelIndex].slopeAngleMin = EditorGUILayout.Slider("Slope Angle Min", _pp.paintAssetsDatabase[_pp.currSelIndex].slopeAngleMin, 0.0f, 90f);
            _pp.paintAssetsDatabase[_pp.currSelIndex].slopeAngleMax = EditorGUILayout.Slider("Slope Angle Max", _pp.paintAssetsDatabase[_pp.currSelIndex].slopeAngleMax, 0.0f, 90f);
        }
        else
        {
            EditorGUI.BeginDisabledGroup(true);
            EditorGUILayout.Slider("Scale Min", 1f, 0.01f, 5f);
            EditorGUILayout.Slider("Scale Max", 1.5f, 0.01f, 5f);
            EditorGUILayout.Toggle("Random Yaw", true);
            EditorGUILayout.Toggle("Align to Normal", true);
            EditorGUILayout.Slider("Slope Angle Min", 0f, 0f, 90f);
            EditorGUILayout.Slider("Slope Angle Max", 45f, 0f, 90f);
            EditorGUI.EndDisabledGroup();
        }
        GUILayout.Space(10);
    }


    public void DropAreaGUI()
    {
        Event evt = Event.current;
        GUILayout.BeginHorizontal(EditorStyles.helpBox);
        dropArea = GUILayoutUtility.GetRect(0.0f, 150.0f, GUILayout.ExpandWidth(true), GUILayout.ExpandHeight(true));
        dropArea.width = EditorGUIUtility.currentViewWidth;
        GUI.Box(dropArea, "Drag objects here");
        AssetsGridview();
        GUILayout.EndHorizontal();

        switch (evt.type)
        {
            case EventType.DragUpdated:
            case EventType.DragPerform:
                if (!dropArea.Contains(evt.mousePosition))
                    return;
                DragAndDrop.visualMode = DragAndDropVisualMode.Copy;

                if (evt.type == EventType.DragPerform)
                {
                    DragAndDrop.AcceptDrag();

                    foreach (Object dragged_object in DragAndDrop.objectReferences)
                    {
                        Debug.Log("DRAGGED " + dragged_object);
                        //Add to saved
                        _pp.AddAsset((GameObject) dragged_object);
                    }
                }
                break;
        }
    }

    private void AssetsGridview()
    {
        int colNum = (int) Mathf.Floor((dropArea.xMax - dropArea.xMin) / (_previewSize + _previewPadding * 2));
        int r = 0;
        int c = 0;
        for (int i = 0; i < _pp.paintAssetsDatabase.Count; i++)
        {
            PaintAsset asset = _pp.GetAsset(i);
            bool disable = false;
            if (!asset.isChecked) disable = true;
            
            //Asset preview
            if(disable) EditorGUI.BeginDisabledGroup(true);
            bool assetClicked = GUI.Toggle(new Rect(dropArea.x + c * _previewSize + _previewPadding + (c + 1) * 15,
                            dropArea.y + r * _previewSize + _previewPadding,
                            _previewSize,
                            _previewSize), asset.isSelected,
                            new GUIContent(AssetPreview.GetAssetPreview(asset.asset), asset.asset.name), GUI.skin.button);
            if (assetClicked)
            {
                _pp.Select(i);
            }
            if (disable) EditorGUI.EndDisabledGroup();
            
            //Add asset to selected group
            bool assetChecked = GUI.Toggle(new Rect(dropArea.x + c * 90 + _previewPadding,
                                dropArea.y + r * _previewSize + _previewPadding,
                                10, 10), asset.isChecked, "");
            if (assetChecked != asset.isChecked)
            {
                _pp.ToggleAsset(i);
            }

            //Layout increment
            if (++c >= colNum)
            {
                c = 0;
                r++;
            }
        }
    }

    public void OnDestroy() => _pp.SaveAllAsset();
}
