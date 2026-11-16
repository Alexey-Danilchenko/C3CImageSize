/**********************************************************************************

 C3C Image Size script for representing C3C Image Size plugin in Filter menu

 To install this JavaScript, place it in the Photoshop Scripts folder
 (Photoshop XXXXXXX/Presets/Scripts)

**********************************************************************************/

<javascriptresource>
<name>C3C Image Size...</name>
<menu>filter</menu>
<enableinfo>true</enableinfo>
</javascriptresource>

var desc = new ActionDescriptor();
var idpCthreeW = charIDToTypeID("pC3W");
desc.putInteger(idpCthreeW, 900);
var scripting = charIDToTypeID("cC3S");
desc.putBoolean(scripting, true);
try {
    executeAction(stringIDToTypeID("5f3d6843-d895-44c2-bbb7-b71ca0cb8c01"), desc, DialogModes.ALL);
} catch (e) {
    // Error 8007 = "User cancelled the operation"
    if (e.number === 8007) {
        // Do nothing: exit silently and cleanly
    } else {
        // Re-throw or alert actual bugs so you don't hide real errors
        alert("Script Error (" + e.number + "): " + e.message);
    }
}









