using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PaintAsset
{
    public bool randomYaw;
    public bool alignToNormal;
    public float slopeAngleMin;
    public float slopeAngleMax;
    public float scaleMin;
    public float scaleMax;
    public bool isSelected;
    public bool isChecked;
    public GameObject asset;

    public PaintAsset(GameObject _asset)
    {
        randomYaw = true;
        alignToNormal = true;
        slopeAngleMin = 0.0f;
        slopeAngleMax = 45.0f;
        scaleMin = 1.0f;
        scaleMax = 1.5f;
        asset = _asset;
        isSelected = false;
        isChecked = true;
    }
}

public class PaintAssetProfile : ScriptableObject
{
    public bool singlePlacement;
    public int density;
    public float radius;
    public List<PaintAsset> paintAssetsDatabase;
}