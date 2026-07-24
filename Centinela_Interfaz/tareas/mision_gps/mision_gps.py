#Codigo para registrar la mision de GPS y guardar los datos en un archivo Excel

from datetime import datetime
from pathlib import Path
from typing import Protocol, List, Dict, Any

from openpyxl import Workbook


class AdministrarMision(Protocol):

    def start(self) -> None:
        ...

    def stop(self) -> None:
        ...

    def add_route_point(self, latitude: float, longitude: float) -> None:
        ...

    def add_waypoint(self, latitude: float, longitude: float) -> None:
        ...

    def save_to_excel(self) -> None:
        ...


class MisionGPS(AdministrarMision):

    def __init__(self):

        self.recording: bool = False
        self.log: List[Dict[str, Any]] = []

    def start(self) -> None:

        self.recording = True
        self.log = []

        print("[MISSION] Recording started.")

    def stop(self) -> None:

        if self.recording:
            self.save_to_excel()

        self.recording = False

        print("[MISSION] Recording stopped.")

    def add_route_point(self, latitude: float, longitude: float) -> None:

        if not self.recording:
            return

        self.log.append({

            "Tiempo": datetime.now().strftime("%H:%M:%S"),
            "Latitud": latitude,
            "Longitud": longitude,
            "Tipo": "Ruta"

        })

    def add_waypoint(self, latitude: float, longitude: float) -> None:

        if not self.recording:
            return

        self.log.append({

            "Tiempo": datetime.now().strftime("%H:%M:%S"),
            "Latitud": latitude,
            "Longitud": longitude,
            "Tipo": "Waypoint"

        })

    def save_to_excel(self) -> None:

        missions_folder = Path("missions")
        missions_folder.mkdir(exist_ok=True)

        wb = Workbook()

        ws = wb.active
        ws.title = "Mission"

        ws.append([
            "Tiempo",
            "Latitud",
            "Longitud",
            "Tipo"
        ])

        for row in self.log:

            ws.append([
                row["Tiempo"],
                row["Latitud"],
                row["Longitud"],
                row["Tipo"]
            ])

        filename = missions_folder / f"Mission_{datetime.now():%Y%m%d_%H%M%S}.xlsx"

        wb.save(filename)

        print(f"[MISSION] Mission saved to {filename}")