pipeline {
    agent { label 'linux' }
    stages {
        stage('Build') {
            steps {
                sh 'sudo pio run'
            }
        }
    }
}