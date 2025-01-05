from django.http import JsonResponse
from django.views.decorators.csrf import csrf_exempt
import json
from .models import SensorData

@csrf_exempt
def sensor_data(request):
    if request.method == "POST":
        try:
            data = json.loads(request.body)
            temperature = data.get("temperature")
            humidity = data.get("humidity")
            SensorData.objects.create(temperature=temperature, humidity=humidity)
            return JsonResponse({"status": "success"}, status=201)
        except Exception as e:
            return JsonResponse({"status": "error", "message": str(e)}, status=400)

    elif request.method == "GET":
        sensor_data = SensorData.objects.all().order_by('-timestamp')[:10]
        response = [{"temperature": d.temperature, "humidity": d.humidity, "timestamp": d.timestamp} for d in sensor_data]
        return JsonResponse(response, safe=False)
