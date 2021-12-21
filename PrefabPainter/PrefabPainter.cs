using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

[ExecuteInEditMode]
public class PrefabPainter : ScriptableObject
{
    public enum MODE
    {
        SELECT, PAINT, EARSE,
    }
    public MODE mode = MODE.PAINT;
    public bool singlePlacement = false;
    
    public int density = 1;
    public float radius = 5f;
    public int currSelIndex = -1;
    //public int count = 0;
    public List<PaintAsset> paintAssetsDatabase = new List<PaintAsset>();
    private List<PaintAsset> currSelected = new List<PaintAsset>();

    public void Paint(Vector3 hitPoint, Vector3 hitNormal)
    {
        if (currSelected.Count == 0) return;
        if (singlePlacement)
        {
            int index = Random.Range(0, currSelected.Count);
            PlaceOneAt(currSelected[index].asset, hitPoint, hitNormal);
            return;
        }
        
        Vector3 hitPoint_random = hitPoint;
        Vector3 hitNormal_random = hitNormal;

        for (int i = 0; i < density; i++)
        {
            int index = Random.Range(0, currSelected.Count);
            // Raycast on to terrain
            Vector2 randomPoint = Random.insideUnitCircle * radius;
            Vector3 rayStart = hitPoint + hitNormal * 5f + new Vector3(randomPoint.x, 0, randomPoint.y);

            Ray ray = new Ray(rayStart, -hitNormal);
            RaycastHit hit;
            if (Physics.Raycast(ray, out hit, 100f))
            {
                hitPoint_random = hit.point;
                hitNormal_random = hit.normal;
                PlaceOneAt(currSelected[index].asset, hitPoint_random, hitNormal_random);
            }
        }
    }

    private void PlaceOneAt(GameObject obj, Vector3 center, Vector3 normal)
    {
        float angle = Vector3.Angle(Vector3.up, normal);
        if (angle >= paintAssetsDatabase[currSelIndex].slopeAngleMin && angle < paintAssetsDatabase[currSelIndex].slopeAngleMax)
        {
            GameObject newObject = Instantiate(obj, center, Quaternion.identity);
            // Apply the random scale
            Vector3 newScale = Vector3.one * Random.Range(paintAssetsDatabase[currSelIndex].scaleMin, paintAssetsDatabase[currSelIndex].scaleMax);
            newObject.transform.localScale = newScale;
            // Align along normal
            if (paintAssetsDatabase[currSelIndex].alignToNormal)
                newObject.transform.up = normal;
            // Random rotation around y
            if (paintAssetsDatabase[currSelIndex].randomYaw)
                newObject.transform.Rotate(new Vector3(0, 1, 0), Random.Range(0, 360));
            Undo.RegisterCreatedObjectUndo(newObject, "paint");
        }
    }

    public void AddAsset(GameObject asset)
    {
        foreach (PaintAsset pa in paintAssetsDatabase)
            if (pa.asset == asset) return;
        paintAssetsDatabase.Add(new PaintAsset(asset));
        UpdateIsSelected(paintAssetsDatabase.Count - 1);
        currSelIndex = paintAssetsDatabase.Count - 1;
    }

    public PaintAsset GetAsset(int index)
    {
        if(index < paintAssetsDatabase.Count)
            return paintAssetsDatabase[index];
        return null;
    }    

    public bool GetIsChecked(int index)
    {
        if (index < paintAssetsDatabase.Count)
            return paintAssetsDatabase[index].isChecked;
        return false;
    }

    public void ToggleAsset(int index)
    {
        if (index < paintAssetsDatabase.Count)
            paintAssetsDatabase[index].isChecked = !paintAssetsDatabase[index].isChecked;
        UpdateIsSelected(index);
    }

    public void Select(int index)
    {
        currSelIndex = index;
        foreach (PaintAsset pa in paintAssetsDatabase)
            pa.isSelected = false;
        paintAssetsDatabase[index].isSelected = true;
    }

    public void RemoveSelected()
    {
        if (currSelIndex < 0 || currSelIndex >= paintAssetsDatabase.Count) return;
        paintAssetsDatabase.RemoveAt(currSelIndex);
        currSelIndex = Mathf.Max(0, currSelIndex - 1);
    }

    public void UpdateIsSelected(int index)
    {
        if (paintAssetsDatabase[index].isChecked && !currSelected.Contains(paintAssetsDatabase[index]))
            currSelected.Add(paintAssetsDatabase[index]);
        else if (!paintAssetsDatabase[index].isChecked && currSelected.Contains(paintAssetsDatabase[index]))
            currSelected.Remove(paintAssetsDatabase[index]);
    }

    //Save assets and settings
    public void SaveAllAsset()
    {
        PaintAssetProfile profile = new PaintAssetProfile();
        profile.paintAssetsDatabase = paintAssetsDatabase;
        profile.singlePlacement = singlePlacement;
        profile.density = density;
        profile.radius = radius;
        AssetDatabase.CreateAsset(profile, "Assets/Editor/PrefabPainterProfile.asset");
        AssetDatabase.SaveAssets();
    }

    //Load assets and settings
    public void LoadAllAssets()
    {
        PaintAssetProfile profile = (PaintAssetProfile) AssetDatabase.LoadAssetAtPath("Assets/Editor/PrefabPainterProfile.asset", typeof(PaintAssetProfile));
        if (profile && profile.paintAssetsDatabase != null)
        {
            singlePlacement = profile.singlePlacement;
            density = profile.density;
            radius = profile.radius;
            paintAssetsDatabase = profile.paintAssetsDatabase;
            foreach (PaintAsset pa in profile.paintAssetsDatabase)
                if (pa.isChecked)
                    currSelected.Add(pa);
            if (profile.paintAssetsDatabase.Count > 0) currSelIndex = 0;
        }
    }
}
