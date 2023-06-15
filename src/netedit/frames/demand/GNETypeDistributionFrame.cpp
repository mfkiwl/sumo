/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2023 German Aerospace Center (DLR) and others.
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0/
// This Source Code may also be made available under the following Secondary
// Licenses when the conditions for such availability set forth in the Eclipse
// Public License 2.0 are satisfied: GNU General Public License, version 2
// or later which is available at
// https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
/****************************************************************************/
/// @file    GNETypeDistributionFrame.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jun 2023
///
// The Widget for edit type distribution elements
/****************************************************************************/
#include <config.h>

#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/changes/GNEChange_DemandElement.h>
#include <netedit/elements/demand/GNEVType.h>
#include <netedit/dialogs/GNEVehicleTypeDialog.h>
#include <netedit/dialogs/GNEVTypeDistributionsDialog.h>
#include <utils/gui/div/GUIDesigns.h>
#include <utils/gui/windows/GUIAppEnum.h>

#include "GNETypeDistributionFrame.h"


// ===========================================================================
// FOX callback mapping
// ===========================================================================

FXDEFMAP(GNETypeDistributionFrame::TypeEditor) typeEditorMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_CREATE,    GNETypeDistributionFrame::TypeEditor::onCmdCreateType),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_DELETE,    GNETypeDistributionFrame::TypeEditor::onCmdDeleteType),
    FXMAPFUNC(SEL_UPDATE,   MID_GNE_DELETE,    GNETypeDistributionFrame::TypeEditor::onUpdDeleteType),
};

FXDEFMAP(GNETypeDistributionFrame::TypeSelector) typeSelectorMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_TYPE,   GNETypeDistributionFrame::TypeSelector::onCmdSelectItem)
};

// Object implementation
FXIMPLEMENT(GNETypeDistributionFrame::TypeEditor,   MFXGroupBoxModule,  typeEditorMap,      ARRAYNUMBER(typeEditorMap))
FXIMPLEMENT(GNETypeDistributionFrame::TypeSelector, MFXGroupBoxModule,  typeSelectorMap,    ARRAYNUMBER(typeSelectorMap))


// ===========================================================================
// method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// GNETypeFrame::TypeEditor - methods
// ---------------------------------------------------------------------------

GNETypeDistributionFrame::TypeEditor::TypeEditor(GNETypeDistributionFrame* typeDistributionFrameParent) :
    MFXGroupBoxModule(typeDistributionFrameParent, TL("Type Editor")),
    myTypeDistributionFrameParent(typeDistributionFrameParent) {
    // Create new vehicle type
    myCreateTypeButton = new FXButton(getCollapsableFrame(), TL("Create type distribution"), GUIIconSubSys::getIcon(GUIIcon::VTYPEDISTRIBUTION), this, MID_GNE_CREATE, GUIDesignButton);
    // Create delete/reset vehicle type
    myDeleteTypeButton = new FXButton(getCollapsableFrame(), TL("Delete type distribution"), GUIIconSubSys::getIcon(GUIIcon::MODEDELETE), this, MID_GNE_DELETE, GUIDesignButton);
    // show type editor
    show();
}


GNETypeDistributionFrame::TypeEditor::~TypeEditor() {}


long
GNETypeDistributionFrame::TypeEditor::onCmdCreateType(FXObject*, FXSelector, void*) {
    auto viewNet = myTypeDistributionFrameParent->myViewNet;
    // obtain a new valid Type ID
    const std::string typeID = viewNet->getNet()->getAttributeCarriers()->generateDemandElementID(SUMO_TAG_VTYPE);
    // create new vehicle type
    GNEDemandElement* type = new GNEVType(viewNet->getNet(), typeID);
    // add it using undoList (to allow undo-redo)
    viewNet->getUndoList()->begin(GUIIcon::VTYPE, "create vehicle type");
    viewNet->getUndoList()->add(new GNEChange_DemandElement(type, true), true);
    viewNet->getUndoList()->end();
    // refresh Type Editor Module
    //myTypeDistributionFrameParent->myTypeEditor->refreshTypeEditorModule();
    return 1;
}


long
GNETypeDistributionFrame::TypeEditor::onCmdDeleteType(FXObject*, FXSelector, void*) {
    auto viewNet = myTypeDistributionFrameParent->myViewNet;
    // begin undo list operation
    viewNet->getUndoList()->begin(GUIIcon::VTYPE, "delete vehicle type");
    // remove vehicle type (and all of their children)
    viewNet->getNet()->deleteDemandElement(myTypeDistributionFrameParent->myTypeSelector->getCurrentType(), viewNet->getUndoList());
    // end undo list operation
    viewNet->getUndoList()->end();
    return 1;
}


long
GNETypeDistributionFrame::TypeEditor::onUpdDeleteType(FXObject* sender, FXSelector, void*) {
    // first check if selected VType is valid
    if (myTypeDistributionFrameParent->myTypeSelector->getCurrentType()) {
        return sender->handle(this, FXSEL(SEL_COMMAND, ID_ENABLE), nullptr);
    } else {
        return sender->handle(this, FXSEL(SEL_COMMAND, ID_DISABLE), nullptr);
    }
}

// ---------------------------------------------------------------------------
// GNETypeFrame::TypeSelector - methods
// ---------------------------------------------------------------------------

GNETypeDistributionFrame::TypeSelector::TypeSelector(GNETypeDistributionFrame* typeFrameParent) :
    MFXGroupBoxModule(typeFrameParent, TL("Current Type")),
    myTypeDistributionFrameParent(typeFrameParent),
    myCurrentType(nullptr) {
    // Create FXComboBox
    myTypeComboBox = new MFXComboBoxIcon(getCollapsableFrame(), GUIDesignComboBoxNCol, true, this, MID_GNE_SET_TYPE, GUIDesignComboBox);
    // add default Types (always first)
    for (const auto& vType : myTypeDistributionFrameParent->getViewNet()->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_VTYPE_DISTRIBUTION)) {
        myTypeComboBox->appendIconItem(vType->getID().c_str(), vType->getACIcon(), FXRGB(255, 255, 200));
    }
    // Set visible items
    if (myTypeComboBox->getNumItems() <= 20) {
        myTypeComboBox->setNumVisible((int)myTypeComboBox->getNumItems());
    } else {
        myTypeComboBox->setNumVisible(20);
    }
    // TypeSelector is always shown
    show();
}


GNETypeDistributionFrame::TypeSelector::~TypeSelector() {}


GNEDemandElement*
GNETypeDistributionFrame::TypeSelector::getCurrentType() const {
    return myCurrentType;
}


void
GNETypeDistributionFrame::TypeSelector::setCurrentType(GNEDemandElement* vType) {
    myCurrentType = vType;
    refreshTypeSelector();
}


void
GNETypeDistributionFrame::TypeSelector::refreshTypeSelector() {
    bool valid = false;
    // clear items
    myTypeComboBox->clearItems();
    // fill myTypeMatchBox with list of type distributions sorted by ID
    std::map<std::string, GNEDemandElement*> types;
    for (const auto& vType : myTypeDistributionFrameParent->getViewNet()->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_VTYPE_DISTRIBUTION)) {
        types[vType->getID()] = vType;
    }
    for (const auto& vType : types) {
        myTypeComboBox->appendIconItem(vType.first.c_str(), vType.second->getACIcon());
    }
    // Set visible items
    if (myTypeComboBox->getNumItems() <= 20) {
        myTypeComboBox->setNumVisible((int)myTypeComboBox->getNumItems());
    } else {
        myTypeComboBox->setNumVisible(20);
    }
    // make sure that tag is in myTypeMatchBox
    if (myCurrentType) {
        for (int i = 0; i < (int)myTypeComboBox->getNumItems(); i++) {
            if (myTypeComboBox->getItem(i).text() == myCurrentType->getID()) {
                myTypeComboBox->setCurrentItem(i);
                valid = true;
            }
        }
    }
    // Check that give vType type is valid
    if (!valid) {
        // set DEFAULT_VEHTYPE as default VType
        myCurrentType = myTypeDistributionFrameParent->getViewNet()->getNet()->getAttributeCarriers()->retrieveDemandElement(SUMO_TAG_VTYPE, DEFAULT_VTYPE_ID);
        // refresh myTypeMatchBox again
        for (int i = 0; i < (int)myTypeComboBox->getNumItems(); i++) {
            if (myTypeComboBox->getItem(i).text() == myCurrentType->getID()) {
                myTypeComboBox->setCurrentItem(i);
            }
        }
    }
    // set myCurrentType as inspected element
    myTypeDistributionFrameParent->getViewNet()->setInspectedAttributeCarriers({myCurrentType});
    // show modules
    myTypeDistributionFrameParent->myTypeAttributesEditor->showAttributeEditorModule(false, true);
    myTypeDistributionFrameParent->myAttributesEditorExtended->showAttributesEditorExtendedModule();
    myTypeDistributionFrameParent->myParametersEditor->refreshParametersEditor();
}


void
GNETypeDistributionFrame::TypeSelector::refreshTypeSelectorIDs() {
    if (myCurrentType) {
        myTypeComboBox->setIconItem(myTypeComboBox->getCurrentItem(), myCurrentType->getID().c_str(), myCurrentType->getACIcon());
    }
}


long
GNETypeDistributionFrame::TypeSelector::onCmdSelectItem(FXObject*, FXSelector, void*) {
    auto viewNet = myTypeDistributionFrameParent->getViewNet();
    // Check if value of myTypeMatchBox correspond of an allowed additional tags
    for (const auto& vType : myTypeDistributionFrameParent->getViewNet()->getNet()->getAttributeCarriers()->getDemandElements().at(SUMO_TAG_VTYPE)) {
        if (vType->getID() == myTypeComboBox->getText().text()) {
            // set pointer
            myCurrentType = vType;
            // set color of myTypeMatchBox to black (valid)
            myTypeComboBox->setTextColor(FXRGB(0, 0, 0));
            // set myCurrentType as inspected element
            viewNet->setInspectedAttributeCarriers({myCurrentType});
            // show modules if selected item is valid
            myTypeDistributionFrameParent->myTypeAttributesEditor->showAttributeEditorModule(false, true);
            myTypeDistributionFrameParent->myAttributesEditorExtended->showAttributesEditorExtendedModule();
            myTypeDistributionFrameParent->myParametersEditor->refreshParametersEditor();
            // Write Warning in console if we're in testing mode
            WRITE_DEBUG(("Selected item '" + myTypeComboBox->getText() + "' in TypeSelector").text());
            // update viewNet
            viewNet->updateViewNet();
            return 1;
        }
    }
    myCurrentType = nullptr;
    // hide all modules if selected item isn't valid
    myTypeDistributionFrameParent->myTypeAttributesEditor->hideAttributesEditorModule();
    myTypeDistributionFrameParent->myAttributesEditorExtended->hideAttributesEditorExtendedModule();
    // set color of myTypeMatchBox to red (invalid)
    myTypeComboBox->setTextColor(FXRGB(255, 0, 0));
    // Write Warning in console if we're in testing mode
    WRITE_DEBUG("Selected invalid item in TypeSelector");
    // update viewNet
    viewNet->updateViewNet();
    return 1;
}

// ---------------------------------------------------------------------------
// GNETypeDistributionFrame - methods
// ---------------------------------------------------------------------------

GNETypeDistributionFrame::GNETypeDistributionFrame(GNEViewParent* viewParent, GNEViewNet* viewNet) :
    GNEFrame(viewParent, viewNet, "Type Distributions") {

    // create type editor
    myTypeEditor = new TypeEditor(this);

    // create type selector
    myTypeSelector = new TypeSelector(this);

    // Create vehicle type attributes editor
    myTypeAttributesEditor = new GNEFrameAttributeModules::AttributesEditor(this);

    // create module for open extended attributes dialog
    myAttributesEditorExtended = new GNEFrameAttributeModules::AttributesEditorExtended(this);
}


GNETypeDistributionFrame::~GNETypeDistributionFrame() {}


void
GNETypeDistributionFrame::show() {
    // show frame
    GNEFrame::show();
}


void
GNETypeDistributionFrame::attributeUpdated(SumoXMLAttr /*attribute*/) {

}

/****************************************************************************/