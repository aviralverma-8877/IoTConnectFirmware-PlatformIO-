pipeline {
    agent { label 'linux' }
    stages {
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
        stage('Build thing') {
            steps {
                sh 'sudo pio run -e thing'
            }
        }
    }
}