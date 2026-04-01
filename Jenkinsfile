pipeline {
    agent none

    options {
        buildDiscarder(logRotator(numToKeepStr: '10'))
        timestamps()
    }

    stages {
        stage('Build') {
            parallel {

                // ─────────────────────────────────────────────
                // Windows – Release & Debug (Win32 / MSBuild)
                // ─────────────────────────────────────────────
                stage('Windows Release') {
                    agent { label 'windows' }
                    steps {
                        checkout scm
                        bat 'msbuild projects\\vs2019\\projects.sln -t:rebuild -property:Configuration=Release -property:Platform=Win32 -maxcpucount:2 -nologo'
                    }
                    post {
                        success {
                            archiveArtifacts artifacts: [
                                'projects/vs2019/Release/hldll/hl.dll',
                                'projects/vs2019/Release/hldll/hl.pdb',
                                'projects/vs2019/Release/hl_cdll/client.dll',
                                'projects/vs2019/Release/hl_cdll/client.pdb'
                            ].join(','), fingerprint: true
                        }
                    }
                }

                stage('Windows Debug') {
                    agent { label 'windows' }
                    steps {
                        checkout scm
                        bat 'msbuild projects\\vs2019\\projects.sln -t:rebuild -property:Configuration=Debug -property:Platform=Win32 -maxcpucount:2 -nologo'
                    }
                    post {
                        success {
                            archiveArtifacts artifacts: [
                                'projects/vs2019/Debug/hldll/hl.dll',
                                'projects/vs2019/Debug/hldll/hl.pdb',
                                'projects/vs2019/Debug/hl_cdll/client.dll',
                                'projects/vs2019/Debug/hl_cdll/client.pdb'
                            ].join(','), fingerprint: true
                        }
                    }
                }

                // ─────────────────────────────────────────────
                // Linux – release & debug (x86 / GCC Makefile)
                // ─────────────────────────────────────────────
                stage('Linux Release') {
                    agent { label 'linux' }
                    steps {
                        checkout scm
                        sh '''
                            sudo apt-get update -qq
                            sudo apt-get install -y g++-multilib libgl1-mesa-dev
                        '''
                        sh 'cd linux && make COMPILER=g++ CFG=release -j$(nproc)'
                    }
                    post {
                        success {
                            archiveArtifacts artifacts: [
                                'linux/release/hl.so',
                                'linux/release/hl.so.dbg',
                                'linux/release/client.so',
                                'linux/release/client.so.dbg'
                            ].join(','), fingerprint: true
                        }
                    }
                }

                stage('Linux Debug') {
                    agent { label 'linux' }
                    steps {
                        checkout scm
                        sh '''
                            sudo apt-get update -qq
                            sudo apt-get install -y g++-multilib libgl1-mesa-dev
                        '''
                        sh 'cd linux && make COMPILER=g++ CFG=debug -j$(nproc)'
                    }
                    post {
                        success {
                            archiveArtifacts artifacts: [
                                'linux/debug/hl.so',
                                'linux/debug/hl.so.dbg',
                                'linux/debug/client.so',
                                'linux/debug/client.so.dbg'
                            ].join(','), fingerprint: true
                        }
                    }
                }

            } // parallel
        } // stage('Build')
    } // stages

    post {
        always {
            echo "Build finished: ${currentBuild.currentResult}"
        }
    }
}
