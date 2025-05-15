using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DomainModels
{
    public class PetData
    {
        public int PetDataId { get; set; }
        public int Hunger { get; set; }
        public int Happiness { get; set; }
        public int Tiredness { get; set; }
        public DateTime Timestamp { get; set; } = DateTime.UtcNow;
    }
}
