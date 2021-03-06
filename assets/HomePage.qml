/*
 * Copyright (c) 2012 SSP Europe GmbH, Munich
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */import bb.cascades 1.0
import bb.system 1.0

/*
 * 
 * Author: Ekkehard Gentz (ekke), Rosenheim, Germany
 * 
 */

Page {
    id: homePage

    //
    attachedObjects: [
        // we need this image as background for the HomeScreen
        // because we toggle often we use an ImageTracker
        // TODO: different background images for different sizes of smartphone or tablet
        ImageTracker {
            id: backgroundLandscape
            imageSource: ods.isSquare()? "asset:///images/login-ods-720x720-o.png" : "asset:///images/login-ods-1280x768-o.png"
        },
        // the BackgroundImage
        ImageTracker {
            id: backgroundPortrait
            imageSource: ods.isPassport()? "asset:///images/login-ods-1440x1440-o.png" :  "asset:///images/login-ods-768x1280-o.png"
        },
        // application supports changing the Orientation
        OrientationHandler {
            onOrientationAboutToChange: {
                if(ods.isPassport()){
                    return 
                }
                if (orientation == UIOrientation.Landscape) {
                    backgroundImage.image = backgroundLandscape.image
                } else {
                    backgroundImage.image = backgroundPortrait.image
                }
            }
        }
    ]

    // the content of the HomeScreen
    content:

    // at first we need a Container with the Background Image
 // we use AbsoluteLayout because then its easy to place the main container at 0,0
 Container {
        id: backgroundContainer
        layout: AbsoluteLayout {
        }
        // the background Image
        ImageView {
            id: backgroundImage
            // image will be set:
            // a: onCreationCompleted
            // b: onUiOrientationChanged
        }
        // the main Container (starting at 0,0)
        Container {
            id: mainContainer
            layoutProperties: AbsoluteLayoutProperties {
                positionX: 0
                positionY: 0
            }
            // place recently used Room here
        }
        // a delayed-for-500 ms Animation to open LoginSheet first time
        // without delay the slide-in was too fast
        animations: [
            TranslateTransition {
                id: startupDelayToOpenSheetFirstTime
                delay: 500
                onEnded: {
                    loginSheet.open();
                }
            }
        ]
    }
    // starts the LoginSheet with some delay
    function loginDelayed() {
        startupDelayToOpenSheetFirstTime.play();
    }
    onCreationCompleted: {
        // Overlay ActionBar
        homePage.actionBarVisibility = ChromeVisibility.Overlay
        // support all orientations
        //OrientationSupport.supportedDisplayOrientation = SupportedDisplayOrientation.All;
        // test current Orientation and set the Background Image
        if(ods.isPassport()){
            backgroundImage.image = backgroundPortrait.image
            return 
        }
        if (ods.isSquare() || OrientationSupport.orientation == UIOrientation.Landscape) {
            backgroundImage.image = backgroundLandscape.image
        } else {
            backgroundImage.image = backgroundPortrait.image
        }
    }
}// end homePage
