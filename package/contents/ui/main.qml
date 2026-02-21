import QtQuick
import QtQuick.Particles
import org.kde.kwin

SceneEffect {
    id: effect
    visible: true

    delegate: Item {
        id: root
        anchors.fill: parent

        // Render all windows on the current screen to ensure they are visible behind the trail
        Repeater {
            model: WindowModel {}
            delegate: WindowThumbnail {
                client: model.window
                visible: model.window.visible && !model.window.minimized && (model.window.output === SceneView.screen)
                x: model.window.frameGeometry.x - SceneView.screen.geometry.x
                y: model.window.frameGeometry.y - SceneView.screen.geometry.y
                width: model.window.frameGeometry.width
                height: model.window.frameGeometry.height
                opacity: model.window.opacity
            }
        }

        ParticleSystem {
            id: sys
            running: true
        }

        // --- Main Trail ---
        ImageParticle {
            system: sys
            id: mainParticle
            source: "../assets/dot.svg"
            color: effect.configuration.TrailColor
            colorVariation: effect.configuration.GradientMode ? 0.5 : 0.0
            
            SequentialAnimation on color {
                running: effect.configuration.RainbowMode
                loops: Animation.Infinite
                ColorAnimation { from: "red"; to: "yellow"; duration: 2000 }
                ColorAnimation { from: "yellow"; to: "green"; duration: 2000 }
                ColorAnimation { from: "green"; to: "cyan"; duration: 2000 }
                ColorAnimation { from: "cyan"; to: "blue"; duration: 2000 }
                ColorAnimation { from: "blue"; to: "magenta"; duration: 2000 }
                ColorAnimation { from: "magenta"; to: "red"; duration: 2000 }
            }
        }

        Emitter {
            id: mainEmitter
            system: sys
            emitRate: 150
            lifeSpan: effect.configuration.TrailLifeSpan
            size: effect.configuration.TrailSize
            sizeVariation: effect.configuration.TrailSize / 4
            
            velocity: AngleDirection {
                angleVariation: 360
                magnitude: 10
                magnitudeVariation: 5
            }

            // Bind to mouse position
            x: workspace.cursorPos.x - SceneView.screen.geometry.x
            y: workspace.cursorPos.y - SceneView.screen.geometry.y
            width: 1
            height: 1
        }

        // --- Sparkle Effect ---
        ImageParticle {
            system: sys
            id: sparkleParticle
            source: "../assets/star.svg"
            color: effect.configuration.SparkleColor
            visible: effect.configuration.SparkleMode
            alpha: 0
            opacity: 1.0
            entryEffect: ImageParticle.Fade
        }

        Emitter {
            id: sparkleEmitter
            system: sys
            enabled: effect.configuration.SparkleMode
            emitRate: 30
            lifeSpan: 600
            size: 16
            sizeVariation: 8
            
            velocity: AngleDirection {
                angleVariation: 360
                magnitude: 50
                magnitudeVariation: 30
            }

            x: workspace.cursorPos.x - SceneView.screen.geometry.x
            y: workspace.cursorPos.y - SceneView.screen.geometry.y
            width: 1
            height: 1
        }

        Component.onCompleted: console.log("Mouse Smear Effect initialized on screen: " + SceneView.screen.name)
    }
}
