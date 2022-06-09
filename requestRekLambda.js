import boto3
import json


def compare_faces(bucket, key):

    client=boto3.client('rekognition')

    response=client.compare_faces(SimilarityThreshold=80,
        SourceImage={
            "S3Object": {
                "Bucket": bucket,
                "Name": key
            }
        },
        TargetImage={
            "S3Object": {
                "Bucket": bucket,
                "Name": "testJH.jpg"
            }
        }
    )
    
    for faceMatch in response['FaceMatches']:
    #position = faceMatch['Face']['BoundingBox']
        similarity = str(faceMatch['Similarity'])
        #print(' matches with ' + similarity + '% confidence')
           
    return len(response['FaceMatches'])
    
    #for faceMatch in response['FaceMatches']:
    #   position = faceMatch['Face']['BoundingBox']
     #   similarity += float(faceMatch['Similarity'])
      #  print("--------------")
       # print(similarity)
        #return similarity


def lambda_handler(event, context):
    results = ''
    mqtt = boto3.client('iot-data', region_name='ap-northeast-2')

    bucket_name = 'esp32-rekognition-148449518658'
    file_name = str(event['payload'])

    res= compare_faces(bucket_name, file_name)
    #if (res > 85):
    #    results += "match"
    print("************")
    print(str(res))
    #results += str(res)
    if(str(res)=='1'):
        print('match!')
        results+='100'
    elif(str(res)=='0'):
        print('mismatch..')
        results+='0'


    response = mqtt.publish(
        topic='esp32/sub/data',
        qos=0,
        payload=results
    )
