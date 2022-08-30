pipeline {
    agent { label 'linux' }
    stages {
        stage('clean') {
            steps {
                sh 'sudo rm -r -f .pio'
                sh 'sudo rm -r -f build'
            }
        }
        stage('Build esp01_1m') {
            steps {
                sh 'sudo pio run -e esp01_1m'
            }
        }
        stage('Build esp12e') {
            steps {
                sh 'sudo pio run -e esp12e'
            }
        }
        stage('Build nodemcuv2') {
            steps {
                sh 'sudo pio run -e nodemcuv2'
            }
        }
        stage('Build huzzah') {
            steps {
                sh 'sudo pio run -e huzzah'
            }
        }
        stage('Create Output') {
            steps {
                sh 'sudo mkdir -p build/esp01_1m'
                sh 'sudo mkdir -p build/esp12e'
                sh 'sudo mkdir -p build/nodemcuv2'
                sh 'sudo mkdir -p build/huzzah'

                sh 'cp .pio/build/esp01_1m/firmware.bin build/esp01_1m/firmware.bin'
                sh 'cp .pio/build/esp12e/firmware.bin build/esp12e/firmware.bin'
                sh 'cp .pio/build/nodemcuv2/firmware.bin build/nodemcuv2/firmware.bin'
                sh 'cp .pio/build/huzzah/firmware.bin build/huzzah/firmware.bin'
            }
        }
    }
}