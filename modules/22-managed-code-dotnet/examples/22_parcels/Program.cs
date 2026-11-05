// Module 22: a small managed program used to study metadata, IL and decompiler output.
// Build: dotnet build -c Release -o build   (see the Makefile)
using System;

namespace Parcels
{
    public enum Zone
    {
        Local = 1,
        Regional = 2,
        National = 3
    }

    public class Parcel
    {
        private readonly int weightGrams;
        private readonly Zone zone;

        public Parcel(int weightGrams, Zone zone)
        {
            this.weightGrams = weightGrams;
            this.zone = zone;
        }

        // Cost in cents: a rate per started 500 g, one cent more for every unit after the fourth.
        public int Cost()
        {
            int units = (weightGrams + 499) / 500;
            int rate = zone switch
            {
                Zone.Local => 3,
                Zone.Regional => 5,
                _ => 9
            };
            int total = 0;
            for (int i = 0; i < units; i++)
            {
                total += rate;
                if (i >= 4)
                {
                    total += 1;
                }
            }
            return total;
        }

        public string Label()
        {
            return $"{zone} parcel, {weightGrams} g, cost {Cost()}";
        }
    }

    public static class Routes
    {
        // A switch on a string; the compiler turns it into tests on the length and the characters.
        public static string Region(string code)
        {
            switch (code)
            {
                case "N": return "north";
                case "S": return "south";
                case "E": return "east";
                case "W": return "west";
                case "NE": return "northeast";
                case "NW": return "northwest";
                case "SE": return "southeast";
                default: return "unknown";
            }
        }

        // A lambda that captures a local variable, which the compiler turns into a helper class.
        public static int[] Surcharges(int[] weights, int limit)
        {
            Func<int, int> extra = w => w > limit ? (w - limit) / 100 : 0;
            int[] result = new int[weights.Length];
            for (int i = 0; i < weights.Length; i++)
            {
                result[i] = extra(weights[i]);
            }
            return result;
        }
    }

    public static class Program
    {
        public static int Main(string[] args)
        {
            Parcel[] parcels =
            {
                new Parcel(1200, Zone.Local),
                new Parcel(3100, Zone.Regional),
                new Parcel(500, Zone.National)
            };
            foreach (Parcel p in parcels)
            {
                Console.WriteLine(p.Label());
            }
            Console.WriteLine(Routes.Region("NW"));
            Console.WriteLine(string.Join(",", Routes.Surcharges(new[] { 800, 2500, 4000 }, 1000)));
            return 0;
        }
    }
}
